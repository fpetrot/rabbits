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

#include <cassert>

#include "ui.h"


#include "rabbits-common.h"
#include "rabbits/logger.h"

bool sdl_ui::sdl_inited = false;

void sdl_ui::sdl_cleanup()
{
    SDL_Quit();
    sdl_inited = false;
}

sdl_ui::sdl_ui()
{
    assert(!sdl_ui::sdl_inited);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1) {
        ERR_PRINTF("SDL_Init failed\n");
        goto sdl_init_err;
    }

    m_screen = SDL_SetVideoMode(DEFAULT_W, DEFAULT_H, 0, 0);

    if (m_screen == NULL) {
        ERR_PRINTF("SDL_SetVideoMode failed\n");
        goto sdl_screen_err;
    }

    atexit(sdl_cleanup);
    sdl_inited = true;

    m_active_fb = NULL;

    sdl_screen_err: sdl_init_err: return;
}

sdl_ui::~sdl_ui()
{
}

ui_fb* sdl_ui::new_fb(std::string name, const ui_fb_info &info)
{
    sdl_ui_fb *fb = new sdl_ui_fb(info);

    m_fbs.push_back(fb);

    if (m_active_fb == NULL) {
        show_fb(fb);
    }

    return fb;
}

void sdl_ui::update()
{
    sdl_ui_fb *active_fb = static_cast<sdl_ui_fb*>(m_active_fb);
    if (active_fb) {
        if (SDL_BlitSurface(active_fb->get_surface(), NULL, m_screen, NULL)
                == -1) {
            ERR_PRINTF("blit failed: %s\n", SDL_GetError());
        }
        SDL_Flip(m_screen);
    }
}
