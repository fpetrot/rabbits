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

#ifndef _RABBITS_COMPONENT_CONNECTION_STRATEGY_TLM_INITIATOR_BUS_H
#define _RABBITS_COMPONENT_CONNECTION_STRATEGY_TLM_INITIATOR_BUS_H

#include "rabbits/component/connection_strategy.h"
#include "rabbits/datatypes/tlm.h"
#include "rabbits/datatypes/address_range.h"
#include "rabbits/logger.h"

#include <systemc>

template <unsigned int BUSWIDTH = 32>
class TlmInitiatorBusCS : public ConnectionStrategy< TlmInitiatorBusCS<BUSWIDTH> > {
public:
    typedef sc_core::sc_port<MemoryMappingInspectorScIface, 
            1, sc_core::SC_ZERO_OR_MORE_BOUND> ScPortInspector;
    using typename ConnectionStrategyBase::BindingResult;

private:
    enum kind_e { BUS, INITIATOR };

    typedef typename TlmSocketBase<BUSWIDTH>::Initiator Initiator;
    Initiator *m_initiator = nullptr;

    ScPortInspector *m_inspector = nullptr;

    TlmBusIface<BUSWIDTH> *m_bus = nullptr;

    kind_e m_kind;

public:
    explicit TlmInitiatorBusCS(typename TlmSocketBase<BUSWIDTH>::Initiator &s)
        : m_initiator(&s), m_kind(INITIATOR) {}

    TlmInitiatorBusCS(typename TlmSocketBase<BUSWIDTH>::Initiator &s, ScPortInspector &inspector)
        : m_initiator(&s), m_inspector(&inspector), m_kind(INITIATOR) {}

    explicit TlmInitiatorBusCS(TlmBusIface<BUSWIDTH> &bus)
        : m_bus(&bus), m_kind(BUS) {}

    BindingResult bind_peer(TlmInitiatorBusCS<BUSWIDTH> &cs, PlatformDescription &d)
    {
        if (m_kind == cs.m_kind) {
            const std::string &kind_s = (m_kind == BUS) ? "bus" : "tlm initiator";

            LOG(APP, ERR) << "Cannot bind a " << kind_s << " to a " << kind_s << "\n";
            return BindingResult::BINDING_ERROR;
        }

        TlmBusIface<BUSWIDTH> &bus = (m_kind == BUS) ? *m_bus : *cs.m_bus;
        Initiator &initiator = (m_kind == BUS) ? *cs.m_initiator : *m_initiator;

        bus.connect_initiator(initiator);

        if ((m_kind == INITIATOR) && (m_inspector != nullptr)) {
            m_inspector->bind(bus);
        } else if ((m_kind == BUS) && (cs.m_inspector != nullptr)) {
            cs.m_inspector->bind(bus);
        }

        return BindingResult::BINDING_OK;
    }

    BindingResult bind_hierarchical(TlmInitiatorBusCS<BUSWIDTH> &parent_cs)
    {
        if (m_kind != parent_cs.m_kind) {
            return BindingResult::BINDING_HIERARCHICAL_TYPE_MISMATCH;
        }

        /* Let specific CS handle the binding */
        return BindingResult::BINDING_TRY_NEXT;
    }
};

#endif

