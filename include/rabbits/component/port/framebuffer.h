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

#pragma once

#include "rabbits/component/port.h"
#include "rabbits/component/connection_strategy/framebuffer.h"

class FramebufferOutPort : public Port {
protected:
    FramebufferCS m_cs;

public:
    FramebufferCS::FramebufferOutScPort sc_p;

    FramebufferOutPort(const std::string name)
        : Port(name)
        , m_cs(sc_p)
        , sc_p(name.c_str())
    {
        add_connection_strategy(m_cs);
        declare_parent(sc_p.get_parent_object());
        add_attr_to_parent("framebuffer-out", name);
    }

    void set_info(FramebufferInfo &info) {
        sc_p->set_info(info);
    }

    void set_palette(const std::vector<uint32_t> &p)
    {
        sc_p->set_palette(p);
    }

    void set_backlight_level(uint8_t lvl) {
        sc_p->set_backlight_level(lvl);
    }

    const char * get_typeid() const { return "framebuffer-out"; }
};

class FramebufferInPort : public Port {
protected:
    FramebufferCS m_cs;

public:
    FramebufferCS::FramebufferInScExport sc_e;

    FramebufferInPort(const std::string name, FramebufferSystemCInterface &iface)
        : Port(name)
        , m_cs(sc_e)
        , sc_e(name.c_str())
    {
        sc_e(iface);
        add_connection_strategy(m_cs);
        declare_parent(sc_e.get_parent_object());
        add_attr_to_parent("framebuffer-in", name);
    }

    const char * get_typeid() const { return "framebuffer-in"; }
};
