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

#include <rabbits/ui/view/framebuffer.h>
#include <rabbits/ui/ui.h>

#include "ui.h"

using namespace sc_core;
using std::string;

FramebufferUiBackend::FramebufferUiBackend(sc_module_name n,
                                           const Parameters &p,
                                           ConfigManager &c)
    : Component(n, p, c)
    , p_fb("fb", *this)
{
    FramebufferInfo info;
    string ui_name;

    info.enabled = false;

    if (!p["displayed-name"].is_default()) {
        ui_name = p["displayed-name"].as<string>();
    } else {
        ui_name = string(n);
    }

    m_ui_view = c.get_ui().create_framebuffer(ui_name, info);

    if (!m_ui_view) {
        MLOG(APP, DBG) << "The current user interface does not support framebuffer rendering\n";
    }
}

void FramebufferUiBackend::set_info(const FramebufferInfo &info)
{
    if (m_ui_view) {
        m_ui_view->set_info(info);
    }
}

void FramebufferUiBackend::set_palette(const std::vector<uint32_t> &palette)
{
    if (m_ui_view) {
        m_ui_view->set_palette(palette);
    }
}

void FramebufferUiBackend::set_backlight_level(uint8_t lvl)
{
    if (m_ui_view) {
        m_ui_view->set_backlight_level(lvl);
    }
}
