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

#ifndef _RABBITS_COMPONENT_PORT_TLM_TARGET_H
#define _RABBITS_COMPONENT_PORT_TLM_TARGET_H

#include "rabbits/component/port.h"
#include "rabbits/component/connection_strategy/tlm_initiator_target.h"
#include "rabbits/component/connection_strategy/tlm_target_bus.h"
#include "rabbits/datatypes/address_range.h"

template <unsigned int BUSWIDTH = 32>
class TlmTargetPort : public Port {
public:
    typename TlmInitiatorTargetCS<BUSWIDTH, 1>::target_socket socket;

private:
    TlmInitiatorTargetCS<BUSWIDTH, 1> m_init_target_cs;
    TlmTargetBusCS<BUSWIDTH> m_target_bus_cs;

    void init() {
        add_connection_strategy(m_init_target_cs);
        add_connection_strategy(m_target_bus_cs);
        declare_parent(socket.get_parent_object());
        add_attr_to_parent("tlm-target", "true");
        add_attr_to_parent("tlm-target-port", Port::name());
    }

public:
    TlmTargetPort(const std::string &name, tlm::tlm_fw_transport_if<> &target_iface)
        : Port(name)
        , socket(name.c_str())
        , m_init_target_cs(socket)
        , m_target_bus_cs(socket)
    {
        init();
        socket.bind(target_iface);
    }

    TlmTargetPort(const std::string &name, typename TlmInitiatorTargetCS<BUSWIDTH, 1>::target_socket &target_proxy)
        : Port(name)
        , socket(name.c_str())
        , m_init_target_cs(socket)
        , m_target_bus_cs(socket)
    {
        init();
        socket.bind(target_proxy);
    }

    virtual ~TlmTargetPort() {}

    void register_mapped_ev_listener(TlmTargetMappedListener *l) {
        m_target_bus_cs.register_mapped_ev_listener(l);
    }
};

#endif
