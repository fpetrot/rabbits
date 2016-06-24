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

#ifndef _RABBITS_COMPONENT_PORT_TLM_BUS_H
#define _RABBITS_COMPONENT_PORT_TLM_BUS_H

#include "rabbits/component/port.h"
#include "rabbits/component/connection_strategy/tlm_target_bus.h"
#include "rabbits/component/connection_strategy/tlm_initiator_bus.h"

template <unsigned int BUSWIDTH = 32>
class TlmBusPort : public Port {
private:
    TlmTargetBusCS<BUSWIDTH> m_cs_target;
    TlmInitiatorBusCS<BUSWIDTH> m_cs_init;

public:
    TlmBusPort(const std::string &name, TlmBusIface<BUSWIDTH> &bus)
        : Port(name), m_cs_target(bus), m_cs_init(bus)
    {
        add_connection_strategy(m_cs_target);
        add_connection_strategy(m_cs_init);
        declare_parent(bus.get_sc_module());
        add_attr_to_parent("tlm-bus", "true");
        add_attr_to_parent("tlm-bus-port", Port::name());
    }
};

#endif
