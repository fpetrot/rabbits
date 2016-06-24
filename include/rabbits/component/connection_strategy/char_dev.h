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

#ifndef _RABBITS_COMPONENT_CONNECTION_STRATEGY_CHAR_DEV_H
#define _RABBITS_COMPONENT_CONNECTION_STRATEGY_CHAR_DEV_H

#include <systemc>

#include <rabbits/component/connection_strategy.h>
#include <rabbits/component/channel/char_dev.h>

class CharDeviceCS : public ConnectionStrategy<CharDeviceCS> {
public:
    using typename ConnectionStrategyBase::BindingResult;

private:
    sc_core::sc_port<CharDeviceSystemCInterface> &m_tx;
    sc_core::sc_port<CharDeviceSystemCInterface> &m_rx;

    CharDeviceChannel chan;

public:
    CharDeviceCS(sc_core::sc_port<CharDeviceSystemCInterface> & tx,
                 sc_core::sc_port<CharDeviceSystemCInterface> & rx)
        : m_tx(tx), m_rx(rx) {}

    virtual ~CharDeviceCS() {}

    BindingResult bind_peer(CharDeviceCS &cs, PlatformDescription &d)
    {
        m_tx(chan);
        cs.m_rx(chan);

        m_rx(cs.chan);
        cs.m_tx(cs.chan);

        return BindingResult::BINDING_OK;
    }

    BindingResult bind_hierarchical(CharDeviceCS &parent_cs)
    {
        m_tx(parent_cs.m_tx);
        m_rx(parent_cs.m_rx);

        return BindingResult::BINDING_OK;
    }
};

#endif
