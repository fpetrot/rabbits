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

#ifndef _UI_H
#define _UI_H

#include <string>

#include "ui_fb.h"

class ui
{
private:
    static ui *singleton;

protected:
    ui_fb *m_active_fb;

public:
    virtual ~ui()
    {
    }

    static ui* get_ui();
    static void start_ui();

    virtual ui_fb* new_fb(std::string name, const ui_fb_info &info) = 0;
    virtual void show_fb(ui_fb *fb)
    {
        m_active_fb = fb;
    }

    virtual void update() = 0;
};

#endif
