/*
 *  This file is part of Rabbits
 *  Copyright (C) 2017  Clement Deschamps and Luc Michel
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

#pragma once

#include "rabbits/component/channel/framebuffer.h"
#include "rabbits/component/connection_strategy.h"

class FramebufferCS : public ConnectionStrategy<FramebufferCS> {
public:
    typedef sc_core::sc_port<FramebufferSystemCInterface> FramebufferOutScPort;
    typedef sc_core::sc_export<FramebufferSystemCInterface> FramebufferInScExport;

protected:
    enum eMode { IN, OUT };

    eMode m_mode;

    FramebufferInScExport *m_in = nullptr;
    FramebufferOutScPort *m_out = nullptr;

public:
    FramebufferCS(FramebufferInScExport &in_port)
        : m_mode(IN), m_in(&in_port)
    {}


    FramebufferCS(FramebufferOutScPort &out_port)
        : m_mode(OUT), m_out(&out_port)
    {}

    virtual ~FramebufferCS() {}

    BindingResult bind_peer(FramebufferCS &cs, ConnectionInfo &info, PlatformDescription &d)
    {
        FramebufferInScExport *in = nullptr;
        FramebufferOutScPort *out = nullptr;

        if (m_mode == cs.m_mode) {
            LOG(SIM, WRN) << "Trying to connect together two "
                << ((m_mode == IN) ? "input" : "output") 
                << " framebuffer devices\n";
            return BindingResult::BINDING_ERROR;
        }

        switch (m_mode) {
        case IN:
            in = m_in;
            out = cs.m_out;
            break;

        case OUT:
            in = cs.m_in;
            out = m_out;
            break;
        }

        (*out)(*in);

        return BindingResult::BINDING_OK;
    }

    BindingResult bind_hierarchical(FramebufferCS &parent_cs, ConnectionInfo &info)
    {
        if (m_mode != parent_cs.m_mode) {
            return BindingResult::BINDING_HIERARCHICAL_TYPE_MISMATCH;
        }

        LOG(APP, ERR) << "framebuffer hierarchical binding not supported";
        return BindingResult::BINDING_ERROR;
    }


    virtual const char * get_typeid() const { return "framebuffer"; }
};
