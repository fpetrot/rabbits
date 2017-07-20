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
#ifndef _RABBITS_COMPONENT_CONNECTION_STRATEGY_SPI_H
#define _RABBITS_COMPONENT_CONNECTION_STRATEGY_SPI_H

#include <systemc>
#include <rabbits/component/connection_strategy.h>
#include <rabbits/component/channel/spi.h>

class SpiBindingListener {
public:
    virtual void spi_binding_event(int cs) = 0;
};

class SpiCS : public ConnectionStrategy<SpiCS> {
public:
    using ConnectionStrategyBase::ConnectionInfo;

    typedef sc_core::sc_port<SpiSystemCInterface, 0, sc_core::SC_ZERO_OR_MORE_BOUND> SpiMasterScPort;
    typedef sc_core::sc_export<SpiSystemCInterface> SpiSlaveScExport;

protected:
    enum eMode { MASTER, SLAVE };

    eMode m_mode;

    SpiMasterScPort *m_master_port = nullptr;
    SpiBindingListener *m_master = nullptr;

    SpiSlaveScExport *m_slave = nullptr;

public:
    SpiCS(SpiMasterScPort &master_port, SpiBindingListener &master)
        : m_mode(MASTER), m_master_port(&master_port), m_master(&master)
    {}

    SpiCS(SpiSlaveScExport &slave)
        : m_mode(SLAVE), m_slave(&slave)
    {}

    BindingResult bind_peer(SpiCS &cs, ConnectionInfo &info, PlatformDescription &d)
    {
        SpiMasterScPort *master_port = nullptr;
        SpiBindingListener *master = nullptr;
        SpiSlaveScExport *slave = nullptr;
        int spi_cs;

        if (m_mode == cs.m_mode) {
            LOG(SIM, WRN) << "Trying to connect together two spi "
                << ((m_mode == MASTER) ? "master" : "slave") << " devices\n";
            return BindingResult::BINDING_ERROR;
        }

        if (!d["cs"].is_scalar()) {
            LOG(APP, ERR) << "Missing or invalid chip select `cs' attribute "
                "for SPI binding at " << d.origin() << "\n";
            return BindingResult::BINDING_ERROR;
        }

        try {
            spi_cs = d["cs"].as<int>();
        } catch (PlatformDescription::InvalidConversionException e) {
            LOG(APP, ERR) << "Invalid cs value "
                << d["cs"].as<std::string>()
                << " for SPI binding at "
                << d.origin() << "\n";
            return BindingResult::BINDING_ERROR;
        }

        switch (m_mode) {
        case MASTER:
            master_port = m_master_port;
            master = m_master;
            slave = cs.m_slave;
            break;

        case SLAVE:
            master_port = cs.m_master_port;
            master = cs.m_master;
            slave = m_slave;
            break;
        }

        (*master_port)(*slave);
        master->spi_binding_event(spi_cs);

        info.add("chip select", spi_cs);
        return BindingResult::BINDING_OK;
    }

    BindingResult bind_hierarchical(SpiCS &parent_cs, ConnectionInfo &info)
    {
        if (m_mode != parent_cs.m_mode) {
            return BindingResult::BINDING_HIERARCHICAL_TYPE_MISMATCH;
        }

        LOG(APP, ERR) << "spi hierarchical binding not supported";
        return BindingResult::BINDING_ERROR;
    }

    virtual const char * get_typeid() const { return "spi"; }

    virtual ~SpiCS() {}
};

#endif
