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

#ifndef _RABBITS_COMPONENT_PORT_IN_H
#define _RABBITS_COMPONENT_PORT_IN_H

#include "rabbits/component/port.h"
#include "rabbits/component/connection_strategy/signal.h"

#include <systemc>

template <typename T>
class InPort : public Port {
public:
    sc_core::sc_in<T> sc_p;

private:
    SignalCS<T> m_cs;

    bool m_autoconnect = false;
    sc_core::sc_signal<T> *m_auto_sig = nullptr;
    T m_auto_value;

public:
    InPort(const std::string &name)
        : Port(name), sc_p(name.c_str()), m_cs(sc_p)
    {
        add_connection_strategy(m_cs);
        declare_parent(sc_p.get_parent_object());
    }

    InPort(const std::string &name, sc_core::sc_in<T> &sub_port)
        : Port(name), sc_p(name.c_str()), m_cs(sc_p)
    {
        add_connection_strategy(m_cs);
        declare_parent(sc_p.get_parent_object());
        sub_port(sc_p);
    }

    virtual ~InPort() { delete m_auto_sig; }

    void set_autoconnect_to(const T& value)
    {
        m_autoconnect = true;
        m_auto_value = value;
    }

    void before_end_of_elaboration()
    {
        if ((!is_connected()) && (m_autoconnect)) {
            m_auto_sig = new sc_core::sc_signal<T>;
            sc_p(*m_auto_sig);
            *m_auto_sig = m_auto_value;
        }
    }

};

#endif
