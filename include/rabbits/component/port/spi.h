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
#ifndef _RABBITS_COMPONENT_PORT_SPI_H
#define _RABBITS_COMPONENT_PORT_SPI_H

#include <unordered_map>

#include <rabbits/component/port.h>
#include <rabbits/component/connection_strategy/spi.h>

class SpiMasterPort : public Port, public SpiBindingListener {
protected:
    int m_indexes = 0;
    std::unordered_map<int, int> m_slave_indexes;

    SpiCS m_cs;

    int get_slave_idx(int cs) const
    {
        if (m_slave_indexes.find(cs) == m_slave_indexes.end()) {
            return -1;
        }

        return m_slave_indexes.at(cs);
    }

public:
    SpiCS::SpiMasterScPort sc_p;

    SpiMasterPort(const std::string &name)
        : Port(name)
        , m_cs(sc_p, *this)
        , sc_p(name.c_str())
    {
        add_connection_strategy(m_cs);
        declare_parent(sc_p.get_parent_object());
        add_attr_to_parent("spi-master-port", name);
    }

    void spi_binding_event(int cs)
    {
        m_slave_indexes[cs] = m_indexes++;
    }

    void send(SpiFrame &frame, int cs)
    {
        int slave_idx;

        slave_idx = get_slave_idx(cs);

        if (slave_idx == -1) {
            MLOG_F(SIM, DBG, "Try to send frame to unknown spi device "
                   "at cs %d\n", cs);
            return;
        }

        sc_p[slave_idx]->spi_slave_xmit(frame);

        if (frame.send_data.size() > frame.recv_data.size()) {
            MLOG(SIM, DBG) << "SPI slave did not respond with enough data. Padding.\n";
            for (unsigned int i = frame.recv_data.size(); i < frame.send_data.size(); i++) {
                frame.recv_push(0);
            }
        }
    }
};

class SpiSlavePort : public Port {
protected:
    SpiCS m_cs;

public:
    SpiCS::SpiSlaveScExport sc_e;

    SpiSlavePort(const std::string &name, SpiSystemCInterface &iface)
        : Port(name)
        , m_cs(sc_e)
        , sc_e(name.c_str())
    {
        sc_e(iface);
        add_connection_strategy(m_cs);
        declare_parent(sc_e.get_parent_object());
        add_attr_to_parent("spi-slave-port", name);
    }
};

#endif
