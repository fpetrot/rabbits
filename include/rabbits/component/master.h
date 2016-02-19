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

#ifndef _MASTER_DEVICE_H_
#define _MASTER_DEVICE_H_

#include <systemc>
#include <tlm_utils/simple_initiator_socket.h>

#include "rabbits/logger.h"
#include "rabbits/component/component.h"
#include "rabbits/component/bus.h"

class Master: public Component, public MasterIface
{
protected:
    BusMasterIfaceBase *m_bus_iface;
    bool m_last_bus_access_error;

    void bus_access(tlm::tlm_command cmd, uint64_t addr, uint8_t *data,
                    unsigned int len);

    unsigned int debug_access(tlm::tlm_command cmd, uint64_t addr, uint8_t *data,
                    unsigned int len);

    /* To be overloaded by master devices that want to be informed of memory
     * mapping during end of elaboration */
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

    virtual void bus_read(uint64_t addr, uint8_t *data, unsigned int len);
    virtual void bus_write(uint64_t addr, uint8_t *data, unsigned int len);

    void dmi_hint(uint64_t start, uint64_t size);

    virtual void set_bus_iface(BusMasterIfaceBase *iface) { m_bus_iface = iface; }

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

};

#endif
