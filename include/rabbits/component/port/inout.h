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

#ifndef _RABBITS_COMPONENT_PORT_INOUT_H
#define _RABBITS_COMPONENT_PORT_INOUT_H

#include "rabbits/component/port.h"
#include "rabbits/component/connection_strategy/signal.h"

#include "rabbits/datatypes/typeid.h"

#include <systemc>

template <class T>
class InOutPort : public Port {
public:
    typedef typename sc_core::sc_inout<T>::inout_if_type sc_if_type;

    using Port::bind;
    using Port::operator();

    sc_core::sc_inout<T> sc_p;

    std::string m_typeid = std::string("inout<") + TypeId::get_typeid<T>() + ">";

private:
    SignalCS<T> m_cs;

    bool m_autoconnect = false;
    sc_core::sc_signal<T> *m_auto_sig = nullptr;
    T m_auto_value;

public:
    InOutPort(const std::string &name)
        : Port(name), sc_p(name.c_str()), m_cs(sc_p)
    {
        add_connection_strategy(m_cs);
        declare_parent(sc_p.get_parent_object());
    }

    InOutPort(const std::string &name, sc_core::sc_inout<T> &sub_port)
        : Port(name), sc_p(name.c_str()), m_cs(sc_p)
    {
        add_connection_strategy(m_cs);
        declare_parent(sc_p.get_parent_object());
        sub_port(sc_p);
    }

    virtual ~InOutPort() {}

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

    const char * get_typeid() const { return m_typeid.c_str(); }
};

template <class T>
std::ostream & operator<< (std::ostream &o, const InOutPort<T> &p)
{
    return o << p.sc_p;
}

template <class T>
class InOutMultiPort : public Port {
public:
    typedef typename sc_core::sc_inout<T>::inout_if_type sc_if_type;

    sc_core::sc_port<sc_if_type, 0, sc_core::SC_ZERO_OR_MORE_BOUND> sc_p;

private:
    SignalCS<T> m_cs;

    std::string m_typeid = std::string("inout-multi<") + TypeId::get_typeid<T>() + ">";

public:
    InOutMultiPort(const std::string &name)
        : Port(name), sc_p(name.c_str()), m_cs(sc_p)
    {
        add_connection_strategy(m_cs);
        declare_parent(sc_p.get_parent_object());
    }

    virtual ~InOutMultiPort() {}

    const char * get_typeid() const { return m_typeid.c_str(); }
};

#endif
