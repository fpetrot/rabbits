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

/**
 * @file master.h
 * Master class declaration
 */

#ifndef _MASTER_DEVICE_H_
#define _MASTER_DEVICE_H_

#include <systemc>
#include <tlm_utils/simple_initiator_socket.h>

#include "rabbits/logger.h"
#include "rabbits/component/component.h"
#include "rabbits/component/bus.h"

/**
 * @brief Master (initiator) component on a bus.
 *
 * Represent a component that is connected as a master (a initiator) on a bus.
 */
class Master: public Component, public MasterIface
{
protected:
    BusMasterIfaceBase *m_bus_iface;
    bool m_last_bus_access_error;

    /**
     * @brief Emit a request on the bus.
     *
     * This method emits a TLM request on the bus the master is connected to.
     * Convenient methods bus_read and bus_write are also provided to emit
     * respectively a read or a write request.
     *
     * @param[in] cmd Desired request type (read or write).
     * @param[in] addr Address of the request.
     * @param[in,out] data Array to the data of the request
     * @param[in] len Length of the request.
     *
     * @see bus_read
     * @see bus_write
     */
    void bus_access(tlm::tlm_command cmd, uint64_t addr, uint8_t *data,
                    unsigned int len);


    /**
     * @brief Emit a debug request on the bus.
     *
     * This method emits a TLM debug request on the bus the master is connected
     * to. Such requests are supposed to have no side effects on the slaves
     * such as time consumption.
     *
     * @param[in] cmd Desired request type (read or write).
     * @param[in] addr Address of the request.
     * @param[in,out] data Array to the data of the request
     * @param[in] len Length of the request.
     *
     * @return The number of bytes effectively read or written
     */
    unsigned int debug_access(tlm::tlm_command cmd, uint64_t addr, uint8_t *data,
                    unsigned int len);

    /**
     * @brief DMI information callback.
     *
     * This method is called during end of elaboration if a DMI capable slave
     * is found on the bus. Any master willing to exploit DMI capabilities must
     * override this method to obtain DMI information during end of
     * elaboration.
     *
     * @param[in] start Start of the memory mapping.
     * @param[in] size Size of the memory mapping.
     * @param[in] data Pointer to the start of the data corresponding to the start of the mapping.
     * @param[in] read_latency Time to wait for each read operation.
     * @param[in] write_latency Time to wait for each write operation.
     */
    virtual void dmi_hint_cb(uint64_t start, uint64_t size, void *data,
                             sc_core::sc_time read_latency, sc_core::sc_time write_latency) {}

public:
    Master(sc_core::sc_module_name name)
        : Component(name, ComponentParameters())
        , m_bus_iface(NULL)
        , m_last_bus_access_error(false) {}
    Master(sc_core::sc_module_name name, ComponentParameters &params)
        : Component(name, params)
        , m_bus_iface(NULL)
        , m_last_bus_access_error(false) {}

    virtual ~Master() {}

    /**
     * @brief Emit a read request on the bus the master is connected to.
     *
     * @param[in] addr Address of the read request.
     * @param[in,out] data Array where read result must be written.
     * @param[in] len Length of the read request.
     */
    virtual void bus_read(uint64_t addr, uint8_t *data, unsigned int len);

    /**
     * @brief Emit a write request on the bus the master is connected to.
     *
     * @param[in] addr Address of the write request.
     * @param[in] data Array containing the data of the write request.
     * @param[in] len Length of the write request.
     */
    virtual void bus_write(uint64_t addr, uint8_t *data, unsigned int len);



    /* tlm::tlm_bw_transport_if */
    virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans,
                                               tlm::tlm_phase& phase,
                                               sc_core::sc_time& t)
    {

        ERR_PRINTF("Non-blocking transport not implemented\n");
        abort();
        return tlm::TLM_COMPLETED;
    }

    virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                           sc_dt::uint64 end_range)
    {
        ERR_PRINTF("DMI memory invalidation not implemented\n");
        abort();
    }

    /* MasterIface */
    virtual ComponentBase& get_component() { return *this; }
    void dmi_hint(uint64_t start, uint64_t size);
    virtual void set_bus_iface(BusMasterIfaceBase &iface) { m_bus_iface = &iface; }
    virtual bool bus_iface_is_set() { return m_bus_iface != NULL; }
    BusMasterIfaceBase & get_bus_iface() { return *m_bus_iface; }
};

#endif
