/*
 *  This file is part of Rabbits
 *  Copyright (C) 2015  Clement Deschamps and Luc Michel
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _RABBITS_COMPONENT_PORT_TLM_INITIATOR_H
#define _RABBITS_COMPONENT_PORT_TLM_INITIATOR_H

#include "rabbits/component/port.h"
#include "rabbits/component/connection_strategy/tlm_initiator_target.h"
#include "rabbits/component/connection_strategy/tlm_initiator_bus.h"

template <unsigned int BUSWIDTH = 32>
class TlmInitiatorPort : public Port {
public:
    typename TlmInitiatorTargetCS<BUSWIDTH, 1>::initiator_socket socket;

    sc_core::sc_port<
        MemoryMappingInspectorScIface, 1,
        sc_core::SC_ZERO_OR_MORE_BOUND> inspector;

private:
    enum mode_e { BUS, DIRECT };

    TlmInitiatorTargetCS<BUSWIDTH, 1> m_init_target_cs;
    TlmInitiatorBusCS<BUSWIDTH> m_initiator_bus_cs;
    mode_e m_mode;

    BusAccessResponseStatus m_last_access = BusAccessResponseStatus::OK;

    void init() {
        add_connection_strategy(m_init_target_cs);
        add_connection_strategy(m_initiator_bus_cs);
        declare_parent(socket.get_parent_object());
        add_attr_to_parent("tlm-initiator", "true");
        add_attr_to_parent("tlm-initiator-port", Port::name());
    }

public:

    explicit TlmInitiatorPort(const std::string &name)
        : Port(name)
        , socket(name.c_str())
        , inspector("inspector")
        , m_init_target_cs(socket)
        , m_initiator_bus_cs(socket, inspector)
    {
        init();
    }

    TlmInitiatorPort(const std::string &name,
                     tlm::tlm_bw_transport_if<> &initiator_iface)
        : Port(name)
        , socket(name.c_str())
        , inspector("inspector")
        , m_init_target_cs(socket)
        , m_initiator_bus_cs(socket, inspector)
    {
        init();
        socket.bind(initiator_iface);
    }

    TlmInitiatorPort(const std::string &name,
                     typename TlmInitiatorTargetCS<BUSWIDTH, 1>::initiator_socket &initiator_proxy)
        : Port(name)
        , socket(name.c_str())
        , inspector("inspector")
        , m_init_target_cs(socket)
        , m_initiator_bus_cs(socket, inspector)
    {
        init();
        socket.bind(initiator_proxy);
    }

    virtual ~TlmInitiatorPort() {}

    void selected_strategy(ConnectionStrategyBase &cs)
    {
        if (&cs == &m_init_target_cs) {
            m_mode = DIRECT;
        } else {
            m_mode = BUS;
        }
    }

    void bus_access(tlm::tlm_command cmd, uint64_t addr,
                    uint8_t *data, unsigned int len)
    {
        tlm::tlm_generic_payload trans;

        DBG_PRINTF("bus access: addr=%p, data=%p, len=%d\n",
                   (void *) addr, data, len);

        assert(data);

        sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

        trans.set_command(cmd);
        trans.set_address(addr);
        trans.set_data_ptr(data);
        trans.set_data_length(len);
        trans.set_streaming_width(len);
        trans.set_byte_enable_ptr(NULL);
        trans.set_byte_enable_length(0);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        trans.set_dmi_allowed(false);
        socket->b_transport(trans, delay);

        if (trans.is_response_error()) {
            ERR_PRINTF("Bus %s error at address 0x%.8" PRIx64 ", lenght access: %u byte(s)\n",
                       (cmd == tlm::TLM_READ_COMMAND) ? "read" : "write",
                       addr, len);
        }

        m_last_access = trans.get_response_status();
    }

    unsigned int debug_access(tlm::tlm_command cmd, uint64_t addr, uint8_t *data, unsigned int len)
    {
        tlm::tlm_generic_payload trans;

        DBG_PRINTF("debug access: addr=%p, data=%p, len=%d\n",
                   (void *) addr, data, len);

        assert(data);

        trans.set_command(cmd);
        trans.set_address(addr);
        trans.set_data_ptr(data);
        trans.set_data_length(len);
        return socket->transport_dbg(trans);
    }
    /**
     * @brief Emit a read request on the bus the master is connected to.
     *
     * @param[in] addr Address of the read request.
     * @param[in,out] data Array where read result must be written.
     * @param[in] len Length of the read request.
     */
    void bus_read(uint64_t addr, uint8_t *data, unsigned int len)
    {
        bus_access(tlm::TLM_READ_COMMAND, addr, data, len);
    }

    /**
     * @brief Emit a write request on the bus the master is connected to.
     *
     * @param[in] addr Address of the write request.
     * @param[in] data Array containing the data of the write request.
     * @param[in] len Length of the write request.
     */
    void bus_write(uint64_t addr, uint8_t *data, unsigned int len)
    {
        bus_access(tlm::TLM_WRITE_COMMAND, addr, data, len);
    }


    unsigned int debug_read(uint64_t addr, uint8_t *data, unsigned int len)
    {
        return debug_access(tlm::TLM_READ_COMMAND, addr, data, len);
    }

    unsigned int debug_write(uint64_t addr, uint8_t *data, unsigned int len)
    {
        return debug_access(tlm::TLM_WRITE_COMMAND, addr, data, len);
    }

    const std::vector<AddressRange> & get_memory_mapping()
    {
        return inspector->get_memory_mapping();
    }

    bool dmi_probe(AddressRange range, DmiInfo & info)
    {
        tlm::tlm_generic_payload trans;
        tlm::tlm_dmi dmi_data;

        trans.set_address(static_cast<sc_dt::uint64>(range.begin()));
        trans.set_command(tlm::TLM_READ_COMMAND);

        if (socket->get_direct_mem_ptr(trans, dmi_data)) {
            info.ptr = static_cast<void*>(dmi_data.get_dmi_ptr());

            info.range = AddressRange(dmi_data.get_start_address(),
                                      dmi_data.get_end_address() - dmi_data.get_start_address() + 1);

            info.read_allowed = dmi_data.is_read_allowed();
            info.write_allowed = dmi_data.is_write_allowed();

            info.read_latency = dmi_data.get_read_latency();
            info.write_latency = dmi_data.get_write_latency();

            return true;
        } else {
            return false;
        }
    }

    BusAccessResponseStatus get_last_access_status() const
    {
        return m_last_access;
    }

};

#endif

