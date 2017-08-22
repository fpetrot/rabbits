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

#include "rabbits/test/test.h"
#include "rabbits/component/component.h"
#include "rabbits/component/port/framebuffer.h"

class FramebufferTester : public Component, public FramebufferSystemCInterface {
private:
    bool m_info_is_set = false;
    FramebufferInfo m_info;

    bool m_backlight_lvl_is_set = false;
    uint8_t m_backlight_lvl;

public:
    FramebufferTester(sc_core::sc_module_name n, ConfigManager &c)
        : Component(n, c)
        , p_in("fb", *this)
    {}

    virtual ~FramebufferTester() {}

    void connect_frambuffer_out(ComponentBase &c, const std::string &port = "")
    {
        std::string pname;

        if (!port.empty()) {
            pname = port;
        } else {
            if (!c.has_attr("framebuffer-out")) {
                throw TestFailureException("Missing framebuffer-out attribute "
                                           "on tested component. "
                                           "Please specify the framebuffer "
                                           "output port to connect to");

            }
            pname = c.get_attr("framebuffer-out").front();
        }

        p_in.connect(c.get_port(pname));
    }

    /* Testing interface */
    const FramebufferInfo & get_info()
    {
        if (!m_info_is_set) {
            throw TestFailureException("framebuffer info has not been set");
        }

        m_info_is_set = false;
        return m_info;
    }

    uint8_t get_backlight_level()
    {
        if (!m_backlight_lvl_is_set) {
            throw TestFailureException("backlight level has not been set");
        }

        m_backlight_lvl_is_set = false;
        return m_backlight_lvl;
    }

    /* FramebufferSystemCInterface */
    void set_info(const FramebufferInfo &info)
    {
        if (m_info_is_set) {
            throw TestFailureException("framebuffer info is already set "
                                       "and not read by the testbench yet");
        }

        m_info_is_set = true;
        m_info = info;
    }

    void set_palette(const std::vector<uint32_t> &palette) {}

    void set_backlight_level(uint8_t lvl)
    {
        if (m_backlight_lvl_is_set) {
            throw TestFailureException("backlight level is already set "
                                       "and not read by the testbench yet");
        }

        m_backlight_lvl_is_set = true;
        m_backlight_lvl = lvl;
    }

    FramebufferInPort p_in;
};
