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
    typedef typename sc_core::sc_inout<T>::inout_if_type sc_inout_if_type;
    typedef typename sc_core::sc_in<T>::in_if_type sc_in_if_type;

    typedef typename sc_core::sc_port_b<sc_inout_if_type> sc_inout_p;
    typedef typename sc_core::sc_port_b<sc_in_if_type> sc_in_p;
private:
    enum direction_e {
        IN, INOUT
    };

    sc_core::sc_signal_inout_if<T> *m_sig = nullptr;

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

    virtual ~SignalCS() { delete m_sig; }

    BindingResult bind_peer(SignalCS<T> &cs, PlatformDescription &d)
    {
        if ((m_dir == INOUT) && (cs.m_dir == INOUT)) {
            m_sig = new sc_core::sc_signal<T, sc_core::SC_MANY_WRITERS>;
        } else {
            m_sig = new sc_core::sc_signal<T>;
        }

        bind(*m_sig);
        cs.bind(*m_sig);

        return BindingResult::BINDING_OK;
    }

    BindingResult bind_hierarchical(SignalCS<T> &parent_cs)
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

};

#endif
