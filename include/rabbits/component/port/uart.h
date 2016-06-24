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

#ifndef _RABBITS_COMPONENT_PORT_UART_PORT_H
#define _RABBITS_COMPONENT_PORT_UART_PORT_H

#include <systemc>
#include <cstdlib>

#include <rabbits/component/port.h>
#include <rabbits/component/connection_strategy/char_dev.h>
#include <rabbits/logger.h>

class UartPort : public Port {
public:
    enum eMode { CHAR_DEV, SIGNALS };

private:
    eMode m_mode;
    CharDeviceCS m_chardev_cs;

    sc_core::sc_event m_data_rcv_ev;

public:
    sc_core::sc_port<CharDeviceSystemCInterface> tx, rx;

    UartPort(const std::string & name) : Port(name), m_chardev_cs(tx, rx), tx("tx"), rx("rx")
    {
        add_connection_strategy(m_chardev_cs);
        declare_parent(tx.get_parent_object());
    }

    virtual ~UartPort() {}

    virtual void selected_strategy(ConnectionStrategyBase &cs) {
        if (&cs == &m_chardev_cs) {
            m_mode = CHAR_DEV;
        } else {
            ERR_STREAM("Selected strategy is invalid");
            std::abort();
        }
    }

    const sc_core::sc_event & default_event() const { return rx->default_event(); }

    void recv(std::vector<uint8_t> &data) { rx->recv(data); }
    void send(std::vector<uint8_t> &data) { tx->send(data); }
};
#endif
