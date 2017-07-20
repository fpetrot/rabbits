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
#ifndef _RABBITS_COMPONENT_CONNECTION_STRATEGY_I2C_H
#define _RABBITS_COMPONENT_CONNECTION_STRATEGY_I2C_H

#include <systemc>
#include <rabbits/component/connection_strategy.h>
#include <rabbits/component/channel/i2c.h>

class I2CBindingListener {
public:
    virtual void i2c_binding_event(uint16_t addr) = 0;
};

class I2CCS : public ConnectionStrategy<I2CCS> {
public:
    using typename ConnectionStrategyBase::BindingResult;
    typedef sc_core::sc_port<I2CSystemCInterface, 0, sc_core::SC_ZERO_OR_MORE_BOUND> I2CMasterScPort;
    typedef sc_core::sc_export<I2CSystemCInterface> I2CSlaveScExport;

protected:
    enum eMode { MASTER, SLAVE };

    eMode m_mode;

    I2CMasterScPort *m_master_port = nullptr;
    I2CBindingListener *m_master = nullptr;

    I2CSlaveScExport *m_slave = nullptr;
    uint16_t m_slave_addr;

public:
    I2CCS(I2CMasterScPort &master_port, I2CBindingListener &master)
        : m_mode(MASTER), m_master_port(&master_port), m_master(&master)
    {}

    I2CCS(I2CSlaveScExport &slave, uint16_t addr)
        : m_mode(SLAVE), m_slave(&slave), m_slave_addr(addr)
    {}

    BindingResult bind_peer(I2CCS &cs, PlatformDescription &d)
    {
        I2CMasterScPort *master_port = nullptr;
        I2CBindingListener *master = nullptr;
        I2CSlaveScExport *slave = nullptr;
        uint16_t addr;

        if (m_mode == cs.m_mode) {
            LOG(SIM, WRN) << "Trying to connect together two i2c "
                << ((m_mode == MASTER) ? "master" : "slave") << " devices\n";
            return BindingResult::BINDING_ERROR;
        }

        switch (m_mode) {
        case MASTER:
            master_port = m_master_port;
            master = m_master;
            slave = cs.m_slave;
            addr = cs.m_slave_addr;
            break;

        case SLAVE:
            master_port = cs.m_master_port;
            master = cs.m_master;
            slave = m_slave;
            addr = m_slave_addr;
            break;
        }

        (*master_port)(*slave);
        master->i2c_binding_event(addr);

        return BindingResult::BINDING_OK;
    }

    BindingResult bind_hierarchical(I2CCS &parent_cs)
    {
        if (m_mode != parent_cs.m_mode) {
            return BindingResult::BINDING_HIERARCHICAL_TYPE_MISMATCH;
        }

        LOG(APP, ERR) << "i2c hierarchical binding not supported";
        return BindingResult::BINDING_ERROR;
    }

    virtual const char * get_typeid() const { return "i2c"; }

    virtual ~I2CCS() {}
};

#endif
