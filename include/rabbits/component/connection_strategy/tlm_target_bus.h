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

#ifndef _RABBITS_COMPONENT_CONNECTION_STRATEGY_TLM_TARGET_BUS_H
#define _RABBITS_COMPONENT_CONNECTION_STRATEGY_TLM_TARGET_BUS_H

#include "rabbits/component/connection_strategy.h"
#include "rabbits/datatypes/tlm.h"
#include "rabbits/datatypes/address_range.h"
#include "rabbits/logger.h"

#include <systemc>

class TlmTargetMappedListener {
public:
    virtual void tlm_target_mapped_event(const AddressRange &r) = 0;
};

template <unsigned int BUSWIDTH = 32>
class TlmTargetBusCS : public ConnectionStrategy< TlmTargetBusCS<BUSWIDTH> > {
public:
    using typename ConnectionStrategyBase::BindingResult;
    using typename ConnectionStrategyBase::ConnectionInfo;

private:
    enum kind_e { BUS, TARGET };

    typedef typename TlmSocketBase<BUSWIDTH>::Target Target;
    Target *m_target = nullptr;

    TlmBusIface<BUSWIDTH> *m_bus = nullptr;

    kind_e m_kind;

    std::vector<TlmTargetMappedListener*> m_listeners;

    void mapped_ev_dispatch(const AddressRange &r)
    {
        for (TlmTargetMappedListener *l : m_listeners) {
            l->tlm_target_mapped_event(r);
        }
    }

public:
    explicit TlmTargetBusCS(Target &s) : m_target(&s), m_kind(TARGET) {}
    explicit TlmTargetBusCS(TlmBusIface<BUSWIDTH> &bus) : m_bus(&bus), m_kind(BUS) {}

    void register_mapped_ev_listener(TlmTargetMappedListener *l) {
        m_listeners.push_back(l);
    }

    BindingResult bind_peer(TlmTargetBusCS<BUSWIDTH> &cs, ConnectionInfo &info, PlatformDescription &d)
    {
        AddressRange range;

        if (m_kind == cs.m_kind) {
            const std::string &kind_s = (m_kind == BUS) ? "bus" : "tlm target";

            LOG(APP, ERR) << "Cannot bind a " << kind_s << " to a " << kind_s << " (at " << d.origin() << ")\n";
            return BindingResult::BINDING_ERROR;
        }

        if (!d["address"].is_map()) {
            LOG(APP, ERR) << "Missing address for binding at " << d.origin() << "\n";
            return BindingResult::BINDING_ERROR;
        }

        try {
            range = d["address"].as<AddressRange>();
        } catch(PlatformDescription::InvalidConversionException e) {
            LOG(APP, ERR) << "Invalid address for binding at " << d["address"].origin() << " \n";
            return BindingResult::BINDING_ERROR;
        }

        TlmBusIface<BUSWIDTH> &bus = (m_kind == BUS) ? *m_bus : *cs.m_bus;
        Target &target = (m_kind == BUS) ? *cs.m_target : *m_target;

        bus.connect_target(target, range);
        mapped_ev_dispatch(range);

        info.add("address range", range);

        return BindingResult::BINDING_OK;

    }

    BindingResult bind_hierarchical(TlmTargetBusCS<BUSWIDTH> &parent_cs, ConnectionInfo &info)
    {
        if (m_kind != parent_cs.m_kind) {
            return BindingResult::BINDING_HIERARCHICAL_TYPE_MISMATCH;
        }

        /* Let specific CS handle the binding */
        return BindingResult::BINDING_TRY_NEXT;
    }

    virtual const char * get_typeid() const { return "tlm-target-bus"; }
};

#endif
