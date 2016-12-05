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

#include "rabbits/config.h"
#include "rabbits/ui/ui.h"

#if defined(RABBITS_CONFIG_QT)
# include "qt/ui.h"
#elif defined(RABBITS_CONFIG_SDL)
# include "sdl/ui.h"
#else
# include "dummy/ui.h"
#endif

ui * ui::singleton = NULL;

void ui::create_ui()
{
    if (ui::singleton != NULL) {
        LOG_F(APP, ERR, "ui has already been created\n");
        return;
    }
#if defined(RABBITS_CONFIG_QT)
    ui::singleton = new qt_ui;
#elif defined(RABBITS_CONFIG_SDL)
    ui::singleton = new sdl_ui;
#else
    ui::singleton = new dummy_ui;
#endif
}

ui * ui::get_ui()
{
    return ui::singleton;
}

void ui::dispose_ui()
{
    delete ui::singleton;
}
