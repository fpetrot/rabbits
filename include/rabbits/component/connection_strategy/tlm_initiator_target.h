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

#ifndef _RABBITS_COMPONENT_CONNECTION_STRATEGY_TLM_INITIATOR_TARGET_H
#define _RABBITS_COMPONENT_CONNECTION_STRATEGY_TLM_INITIATOR_TARGET_H

#include <rabbits/component/connection_strategy.h>
#include <rabbits/logger.h>

#include <tlm>

class TlmInitiatorTargetBaseCS : public ConnectionStrategy<TlmInitiatorTargetBaseCS> {
public:
    virtual ~TlmInitiatorTargetBaseCS() {}

    virtual unsigned int get_bus_width() const = 0;

    virtual BindingResult bind_peer(TlmInitiatorTargetBaseCS &cs, PlatformDescription &d) = 0;
    virtual BindingResult bind_hierarchical(TlmInitiatorTargetBaseCS &parent_cs) = 0;
};

template <unsigned int BUSWIDTH = 32>
class TlmInitiatorTargetPortAgnosticCS : public TlmInitiatorTargetBaseCS {
public:
    using typename ConnectionStrategyBase::BindingResult;

    typedef tlm::tlm_base_target_socket_b<BUSWIDTH,
                                          tlm::tlm_fw_transport_if<>,
                                          tlm::tlm_bw_transport_if<> > target_socket_base;

    typedef tlm::tlm_base_initiator_socket_b<BUSWIDTH,
                                             tlm::tlm_fw_transport_if<>,
                                             tlm::tlm_bw_transport_if<> > initiator_socket_base;

protected:
    enum kind_e { TARGET, INITIATOR };

    virtual target_socket_base *get_target_base_socket() = 0;
    virtual initiator_socket_base *get_initiator_base_socket() = 0;
    virtual kind_e get_kind() const = 0;

    virtual void bind(target_socket_base &s) = 0;
    virtual void bind(initiator_socket_base &s) = 0;

    BindingResult bind_peer(TlmInitiatorTargetPortAgnosticCS &cs)
    {
        if (target_socket_base *s = cs.get_target_base_socket()) {
            bind(*s);
        } else if (initiator_socket_base *s = cs.get_initiator_base_socket()) {
            bind(*s);
        }
        
        return BindingResult::BINDING_OK;
    }

    BindingResult bind_hierarchical(TlmInitiatorTargetPortAgnosticCS &parent_cs)
    {
        if (get_kind() != parent_cs.get_kind()) {
            return BindingResult::BINDING_HIERARCHICAL_TYPE_MISMATCH;
        }

        switch (get_kind()) {
        case TARGET:
            parent_cs.bind(*get_target_base_socket());
            break;
        case INITIATOR:
            parent_cs.bind(*get_initiator_base_socket());
            break;
        }

        return BINDING_OK;
    }

public:
    virtual ~TlmInitiatorTargetPortAgnosticCS() {}

    using ConnectionStrategy<TlmInitiatorTargetBaseCS>::bind;

    unsigned int get_bus_width() const { return BUSWIDTH; }

    BindingResult bind_peer(TlmInitiatorTargetBaseCS &cs, PlatformDescription &d)
    {
        if (cs.get_bus_width() != BUSWIDTH) {
            /* XXX Print components name */
            LOG(APP, ERR) << "Unable to connect initiator to target: bus width mismatch.\n";
            return BindingResult::BINDING_ERROR;
        }

        TlmInitiatorTargetPortAgnosticCS<BUSWIDTH> &cs_bw 
            = static_cast<TlmInitiatorTargetPortAgnosticCS<BUSWIDTH>&>(cs);

        return bind_peer(cs_bw);
    }

    BindingResult bind_hierarchical(TlmInitiatorTargetBaseCS &parent_cs)
    {
        if (parent_cs.get_bus_width() != BUSWIDTH) {
            /* XXX Print components name */
            LOG(APP, ERR) << "Unable to connect initiator to target: bus width mismatch.\n";
            return BindingResult::BINDING_ERROR;
        }

        TlmInitiatorTargetPortAgnosticCS<BUSWIDTH> &cs_bw 
            = static_cast<TlmInitiatorTargetPortAgnosticCS<BUSWIDTH>&>(parent_cs);

        return bind_hierarchical(cs_bw);
    }

    virtual const char * get_typeid() const { return "tlm-initiator-target"; }
};

template <unsigned int BUSWIDTH = 32, int N = 1>
class TlmInitiatorTargetCS : public TlmInitiatorTargetPortAgnosticCS<BUSWIDTH> {
public:
    typedef tlm::tlm_target_socket<BUSWIDTH,
                                   tlm::tlm_base_protocol_types,
                                   N> target_socket;
    typedef tlm::tlm_initiator_socket<BUSWIDTH,
                                   tlm::tlm_base_protocol_types,
                                   N> initiator_socket;

    typedef typename TlmInitiatorTargetPortAgnosticCS<BUSWIDTH>::target_socket_base target_socket_base;
    typedef typename TlmInitiatorTargetPortAgnosticCS<BUSWIDTH>::initiator_socket_base initiator_socket_base;

private:
    target_socket *m_target = nullptr;
    initiator_socket *m_initiator = nullptr;
    typedef typename TlmInitiatorTargetPortAgnosticCS<BUSWIDTH>::kind_e kind_e;
    kind_e m_kind;

protected:
    target_socket_base *get_target_base_socket() {
        return m_target;
    }

    initiator_socket_base *get_initiator_base_socket() {
        return m_initiator;
    }

    kind_e get_kind() const { return m_kind; }

    virtual void bind(target_socket_base &s) {
        if (m_target) {
            m_target->bind(s);
        } else if (m_initiator) {
            m_initiator->bind(s);
        }
    }

    virtual void bind(initiator_socket_base &s) {
        if (m_target) {
            m_target->bind(s);
        } else if (m_initiator) {
            m_initiator->bind(s);
        }
    }

public:
    explicit TlmInitiatorTargetCS(target_socket &s)
        : m_target(&s), m_kind(TlmInitiatorTargetPortAgnosticCS<BUSWIDTH>::TARGET) {}
    explicit TlmInitiatorTargetCS(initiator_socket &s)
        : m_initiator(&s), m_kind(TlmInitiatorTargetPortAgnosticCS<BUSWIDTH>::INITIATOR) {}
    virtual ~TlmInitiatorTargetCS() {}

};

#endif
