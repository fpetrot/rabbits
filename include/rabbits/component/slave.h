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

#ifndef _SLAVE_DEVICE_H_
#define _SLAVE_DEVICE_H_

#include <systemc>
#include <tlm>

#include "rabbits/logger.h"

#include "rabbits/component/component.h"

class Slave: public Component, public SlaveIface
{
protected:
    BusSlaveIfaceBase *m_bus_iface;

public:
    Slave(sc_core::sc_module_name name, const ComponentParameters &params);
    Slave(sc_core::sc_module_name name);
    virtual ~Slave();

    virtual void bus_cb_read(uint64_t addr, uint8_t *data, unsigned int len, bool &bErr) {
        switch (len) {
        case 1:
            bus_cb_read_8(addr, (uint8_t *)data, bErr);
            break;
        case 2:
            bus_cb_read_16(addr, (uint16_t *)data, bErr);
            break;
        case 4:
            bus_cb_read_32(addr, (uint32_t *)data, bErr);
            break;
        }
    }

    virtual void bus_cb_read_8(uint64_t addr, uint8_t *value, bool &bErr) {
    	bErr = true;
    }

    virtual void bus_cb_read_16(uint64_t addr, uint16_t *value, bool &bErr) {
    	bErr = true;
    }

    virtual void bus_cb_read_32(uint64_t addr, uint32_t *value, bool &bErr) {
    	bErr = true;
    }

    virtual void bus_cb_write(uint64_t addr, uint8_t *data, unsigned int len, bool &bErr) {
        switch (len) {
        case 1:
            bus_cb_write_8(addr, (uint8_t *)data, bErr);
            break;
        case 2:
            bus_cb_write_16(addr, (uint16_t *)data, bErr);
            break;
        case 4:
            bus_cb_write_32(addr, (uint32_t *)data, bErr);
            break;
        }
    }

    virtual void bus_cb_write_8(uint64_t addr, uint8_t *value, bool &bErr) {
    	bErr = true;
    }

    virtual void bus_cb_write_16(uint64_t addr, uint16_t *value, bool &bErr) {
    	bErr = true;
    }

    virtual void bus_cb_write_32(uint64_t addr, uint32_t *value, bool &bErr) {
    	bErr = true;
    }

    virtual uint64_t debug_read(uint64_t addr, uint8_t* buf, uint64_t size) {
        return 0;
    }

    virtual uint64_t debug_write(uint64_t addr, const uint8_t* buf, uint64_t size) {
        return 0;
    }

    virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                                    tlm::tlm_dmi& dmi_data)
    {
        return false;
    }

    virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans,
                                               tlm::tlm_phase& phase,
                                               sc_core::sc_time& t)
    {
        ERR_PRINTF("Non-blocking transport not implemented\n");
        abort();
        return tlm::TLM_COMPLETED; 
    }

    virtual void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);
    virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans);

    /* SlaveIface */
    virtual ComponentBase& get_component() { return *this; }
    virtual void set_bus_iface(BusSlaveIfaceBase *iface) { m_bus_iface = iface; }
    virtual bool bus_iface_is_set() { return m_bus_iface != NULL; }
    virtual BusSlaveIfaceBase & get_bus_iface() { return *m_bus_iface; }

};


#endif
