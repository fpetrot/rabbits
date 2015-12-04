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

#ifndef _UI_DUMMY_UI
#define _UI_DUMMY_UI

#include <vector>

#include "rabbits/ui/ui.h"
#include "ui_fb.h"

class dummy_ui: public ui
{
private:
    std::vector<dummy_ui_fb*> m_fbs;

protected:
    dummy_ui();

public:
    friend class ui;
    virtual ~dummy_ui();

    ui_fb* new_fb(std::string name, const ui_fb_info &info);

    void update();
};

#endif
