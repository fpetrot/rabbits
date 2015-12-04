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

#ifndef _UI_FB_H
#define _UI_FB_H

enum ui_fb_mode_e
{
    FB_MODE_RGB888 = 0,
    FB_MODE_RGB565,
};

struct ui_fb_info
{
    int physical_w, physical_h;
    int virtual_w, virtual_h;
    int pitch;
    int x_offset, y_offset;
    ui_fb_mode_e mode;
    void * buf;
};

class ui_fb
{
protected:
    ui_fb_info m_info;

public:
    virtual ~ui_fb()
    {
    }

    ui_fb(const ui_fb_info & info)
    {
    }

    virtual void set_info(const ui_fb_info & info) = 0;
};

#endif
