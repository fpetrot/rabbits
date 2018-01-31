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

#ifndef _RABBITS_COMPONENT_CONNECTION_STRATEGY_SIGNAL_H
#define _RABBITS_COMPONENT_CONNECTION_STRATEGY_SIGNAL_H

#include <systemc>

#include "rabbits/component/connection_strategy.h"
#include "rabbits/logger.h"

template <typename T>
class SignalCS : public ConnectionStrategy< SignalCS<T> > {
public:
    using typename ConnectionStrategyBase::BindingResult;
    using typename ConnectionStrategyBase::ConnectionInfo;

    typedef typename sc_core::sc_inout<T>::inout_if_type sc_inout_if_type;
    typedef typename sc_core::sc_in<T>::in_if_type sc_in_if_type;

    typedef typename sc_core::sc_port_b<sc_inout_if_type> sc_inout_p;
    typedef typename sc_core::sc_port_b<sc_in_if_type> sc_in_p;
private:
    enum direction_e {
        IN, INOUT
    };

    typedef sc_core::sc_signal_inout_if<T> Signal;
    typedef std::shared_ptr<Signal> SignalPtr;

    SignalPtr m_sig = nullptr;

    direction_e m_dir;

    sc_in_p *m_in = nullptr;
    sc_inout_p *m_inout = nullptr;

    void bind(sc_core::sc_signal_inout_if<T> &sig) {
        switch (m_dir) {
        case IN:
            m_in->bind(sig);
            break;
        case INOUT:
            m_inout->bind(sig);
            break;
        }
    }

public:
    explicit SignalCS(sc_in_p & port) : m_dir(IN), m_in(&port) {}
    explicit SignalCS(sc_inout_p & port) : m_dir(INOUT), m_inout(&port) {}

    virtual ~SignalCS() {}

    BindingResult bind_peer(SignalCS<T> &peer, ConnectionInfo &info, PlatformDescription &d)
    {
        SignalPtr sig;

        if (m_sig && peer.m_sig) {
            LOG(APP, ERR) << "Trying to bind two ports that have already been "
                "connected. This is not supported\n";
            return BindingResult::BINDING_ERROR;
        }

        if (m_sig) {
            LOG(APP, TRC) << "Reusing signal " << m_sig << "\n";
            sig = peer.m_sig = m_sig;
            peer.bind(*sig);
        } else if (peer.m_sig) {
            LOG(APP, TRC) << "Reusing signal " << peer.m_sig << "\n";
            sig = m_sig = peer.m_sig;
            bind(*sig);
        } else {
            sig = m_sig = peer.m_sig = SignalPtr(new sc_core::sc_signal<T, sc_core::SC_MANY_WRITERS>);
            LOG(APP, TRC) << "Creating new signal " << peer.m_sig << "\n";
            peer.bind(*sig);
            bind(*sig);
        }

        return BindingResult::BINDING_OK;
    }

    BindingResult bind_hierarchical(SignalCS<T> &parent_cs, ConnectionInfo &info)
    {
        if (m_dir != parent_cs.m_dir) {
            return BindingResult::BINDING_HIERARCHICAL_TYPE_MISMATCH;
        }

        switch (m_dir) {
        case IN:
            (*m_in)(*parent_cs.m_in);
            break;
        case INOUT:
            (*m_inout)(*parent_cs.m_inout);
            break;
        }

        return BindingResult::BINDING_OK;
    }

    using ConnectionStrategy< SignalCS<T> >::bind;

    virtual const char * get_typeid() const { return "signal"; }
};

#endif
