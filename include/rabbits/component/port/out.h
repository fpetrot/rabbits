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

#ifndef _RABBITS_COMPONENT_PORT_OUT_H
#define _RABBITS_COMPONENT_PORT_OUT_H

#include "rabbits/component/port.h"
#include "rabbits/component/connection_strategy/signal.h"

#include <systemc>

template <typename T>
class OutPort : public Port {
public:
    typedef typename sc_core::sc_out<T>::inout_if_type sc_if_type;

    using Port::bind;
    using Port::operator();

    sc_core::sc_out<T> sc_p;

private:
    SignalCS<T> m_cs;

public:
    OutPort(const std::string &name) : Port(name), sc_p(name.c_str()), m_cs(sc_p) {
        add_connection_strategy(m_cs);
        declare_parent(sc_p.get_parent_object());
    }

    virtual ~OutPort() {}

};

#endif
