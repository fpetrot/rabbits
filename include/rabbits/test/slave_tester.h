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

#ifndef _RABBITS_TEST_SLAVE_TESTER_H
#define _RABBITS_TEST_SLAVE_TESTER_H

#include "rabbits/component/master.h"
#include "rabbits/test/test.h"

template <unsigned int BUSWIDTH = 32>
class SlaveTester : public Master<> {
public:
    SlaveTester(sc_core::sc_module_name n, ConfigManager &c) : Master(n, c) {}

    virtual ~SlaveTester() {}

    void connect_slave(ComponentBase &c)
    {
        if (!c.has_attr("tlm-target")) {
            throw TestFailureException("Missing tlm-target attribute "
                                       "on tested component. "
                                       "Can't connect it to the slave tester.");
        }
        
        if (!c.has_attr("tlm-target-port")) {
            throw TestFailureException("Missing tlm-target-port attribute "
                                       "on tested component. "
                                       "Can't connect it to the slave tester.");
        }

        const std::string &pname = c.get_attr("tlm-target-port");

        p_bus.connect(c.get_port(pname));
    }

    void bus_write_u8(uint64_t addr, uint8_t data) {
        Master::bus_write(addr, &data, sizeof(data));
    }

    void bus_write_u16(uint64_t addr, uint16_t data) {
        Master::bus_write(addr, reinterpret_cast<uint8_t*>(&data), sizeof(data));
    }

    void bus_write_u32(uint64_t addr, uint32_t data) {
        Master::bus_write(addr, reinterpret_cast<uint8_t*>(&data), sizeof(data));
    }

    uint8_t bus_read_u8(uint64_t addr) {
        uint8_t data;
        Master::bus_read(addr, &data, sizeof(data));
        return data;
    }

    uint16_t bus_read_u16(uint64_t addr) {
        uint16_t data;
        Master::bus_read(addr, reinterpret_cast<uint8_t*>(&data), sizeof(data));
        return data;
    }

    uint32_t bus_read_u32(uint64_t addr) {
        uint32_t data;
        Master::bus_read(addr, reinterpret_cast<uint8_t*>(&data), sizeof(data));
        return data;
    }

    void debug_access_nofail(tlm::tlm_command cmd, uint64_t addr,
                             uint8_t *data, unsigned int len) {
        unsigned int ret;
        
	ret = p_bus.debug_access(cmd, addr, data, len);

        if (ret != len) {
            throw TestFailureException("Debug access length failed");
        }
    }

    uint32_t debug_read_u32_nofail(uint64_t addr) {
        uint32_t data;
        debug_access_nofail(tlm::TLM_READ_COMMAND, addr,
                            reinterpret_cast<uint8_t*>(&data), sizeof(data));
        return data;
    }

    void debug_write_u32_nofail(uint64_t addr, uint32_t data) {
        debug_access_nofail(tlm::TLM_WRITE_COMMAND, addr,
                            reinterpret_cast<uint8_t*>(&data), sizeof(data));
    }

    bool get_dmi_info(DmiInfo &dmi_info) {
	return p_bus.dmi_probe(AddressRange(0,0), dmi_info);
    }

    bool last_access_failed() { return p_bus.get_last_access_status().is_error(); }
    bool last_access_succeeded() { return !last_access_failed(); }
};

#endif
