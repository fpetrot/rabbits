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

#include "rabbits/component/master.h"

#include "rabbits/logger.h"

using namespace sc_core;

Master::~Master(void)
{
}

void Master::bus_access(tlm::tlm_command cmd, uint64_t addr, uint8_t *data,
                               unsigned int len)
{
    tlm::tlm_generic_payload trans;

    DBG_PRINTF("rw: addr=%p, data=%p, len=%d\n", (void *) addr, data,
            len);

    assert(data);
    assert(m_bus_iface);

    sc_time delay = SC_ZERO_TIME;

    trans.set_command(cmd);
    trans.set_address(addr);
    trans.set_data_ptr(data);
    trans.set_data_length(len);
    trans.set_streaming_width(len);
    trans.set_byte_enable_ptr(NULL);
    trans.set_byte_enable_length(0);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans.set_dmi_allowed(false);
    m_bus_iface->b_transport(trans, delay);

    if (trans.is_response_error()) {
        SC_REPORT_ERROR("Master", "response error from b_transport");
    }
}


unsigned int Master::debug_access(tlm::tlm_command cmd, uint64_t addr, uint8_t *data,
                               unsigned int len)
{
    tlm::tlm_generic_payload trans;

    assert(data);
    assert(m_bus_iface);

    trans.set_command(cmd);
    trans.set_address(addr);
    trans.set_data_ptr(data);
    trans.set_data_length(len);
    return m_bus_iface->transport_dbg(trans);
}

void Master::bus_read(uint64_t addr, uint8_t *data, unsigned int len)
{
    bus_access(tlm::TLM_READ_COMMAND, addr, data, len);
}

void Master::bus_write(uint64_t addr, uint8_t *data, unsigned int len)
{
    bus_access(tlm::TLM_WRITE_COMMAND, addr, data, len);
}

void Master::dmi_hint(uint64_t start, uint64_t size)
{
    tlm::tlm_generic_payload trans;
    tlm::tlm_dmi dmi_data;

    assert(m_bus_iface);

    trans.set_address(static_cast<sc_dt::uint64>(start));
    trans.set_command(tlm::TLM_READ_COMMAND);

    if (m_bus_iface->get_direct_mem_ptr(trans, dmi_data)) {
        if (dmi_data.is_read_write_allowed() 
            && dmi_data.get_start_address() == start 
            && dmi_data.get_end_address() == start + size - 1) {
            /* DMI ok */
            dmi_hint_cb(start, size, static_cast<void*>(dmi_data.get_dmi_ptr()),
                        dmi_data.get_read_latency(), dmi_data.get_write_latency());
        } else {
            /* Not enough DMI requirements, ignore it */
            dmi_hint_cb(start, size, NULL, SC_ZERO_TIME, SC_ZERO_TIME);
        }
    } else {
        /* No DMI here */
        dmi_hint_cb(start, size, NULL, SC_ZERO_TIME, SC_ZERO_TIME);
    }
}
