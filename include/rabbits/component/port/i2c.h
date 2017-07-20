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
#ifndef _RABBITS_COMPONENT_PORT_I2C_H
#define _RABBITS_COMPONENT_PORT_I2C_H

#include <unordered_map>

#include <rabbits/component/port.h>
#include <rabbits/component/connection_strategy/i2c.h>

class I2CMasterPort : public Port, public I2CBindingListener {
protected:
    enum eAddressType {
        ADDR_GENERAL_CALL,
        ADDR_START_BYTE,
        ADDR_CBUS,
        ADDR_OTHER_BUS,
        ADDR_RESERVED,
        ADDR_HS_MODE,
        ADDR_DEVICE_ID,
        ADDR_10BITS,
        ADDR_SLAVE
    };

    int m_indexes = 0;
    std::unordered_map<uint16_t, int> m_slave_indexes;

    I2CCS m_cs;

    eAddressType get_address_type(uint16_t addr, I2CFrame::eDirection dir) const
    {
        switch (addr) {
        case 0:
            return (dir == I2CFrame::WRITE)
                ? ADDR_GENERAL_CALL : ADDR_START_BYTE;
        case 1:
            return ADDR_CBUS;

        case 2:
            return ADDR_OTHER_BUS;

        case 3:
            return ADDR_RESERVED;

        case 4 ... 7:
            return ADDR_HS_MODE;

        case 0x7c ... 0x7f:
            return (dir == I2CFrame::READ)
                ? ADDR_DEVICE_ID : ADDR_SLAVE;

        case 0x78 ... 0x7b:
            return ADDR_10BITS;

        default:
            return ADDR_SLAVE;
        }
    }

    int get_slave_idx(uint16_t addr) const
    {
        if (m_slave_indexes.find(addr) == m_slave_indexes.end()) {
            return -1;
        }

        return m_slave_indexes.at(addr);
    }

public:
    I2CCS::I2CMasterScPort sc_p;

    I2CMasterPort(const std::string &name)
        : Port(name)
        , m_cs(sc_p, *this)
        , sc_p(name.c_str())
    {
        add_connection_strategy(m_cs);
        declare_parent(sc_p.get_parent_object());
        add_attr_to_parent("i2c-master-port", name);
    }

    void i2c_binding_event(uint16_t addr)
    {
        m_slave_indexes[addr] = m_indexes++;
    }

    void send(I2CFrame &frame)
    {
        eAddressType addr_type = get_address_type(frame.addr, frame.direction);
        int slave_idx;

        switch (addr_type) {
        case ADDR_SLAVE:
            slave_idx = get_slave_idx(frame.addr);

            if (slave_idx == -1) {
                MLOG_F(SIM, DBG, "Try to send frame to unknown i2c device "
                       "at address 0x%" PRIx16 "\n", frame.addr);
                return;
            }

            sc_p[slave_idx]->i2c_slave_xmit(frame);
            break;

        default:
            MLOG_F(SIM, DBG, "Unsupported special address " PRIx16 "\n", frame.addr);
        }
    }

    const char * get_typeid() const { return "i2c-master"; }
};

class I2CSlavePort : public Port {
protected:
    I2CCS m_cs;

public:
    I2CCS::I2CSlaveScExport sc_e;

    I2CSlavePort(const std::string &name, I2CSystemCInterface &iface, uint16_t addr)
        : Port(name)
        , m_cs(sc_e, addr)
        , sc_e(name.c_str())
    {
        sc_e(iface);
        add_connection_strategy(m_cs);
        declare_parent(sc_e.get_parent_object());
        add_attr_to_parent("i2c-slave-port", name);
    }

    const char * get_typeid() const { return "i2c-slave"; }
};

#endif
