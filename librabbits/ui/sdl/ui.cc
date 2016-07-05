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

void sdl_ui::sdl_cleanup()
{
    SDL_Quit();
    m_sdl_inited = false;
    m_screen = NULL;
}

sdl_ui::sdl_ui()
{
    m_active_fb = NULL;
    m_sdl_inited = false;
    m_screen = NULL;
}

void sdl_ui::sdl_init()
{
    if (m_sdl_inited) {
        return;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1) {
        LOG(APP, ERR) << "SDL_Init failed\n";
        goto sdl_init_err;
    }

    m_screen = SDL_SetVideoMode(DEFAULT_W, DEFAULT_H, 0, 0);

    if (m_screen == NULL) {
        LOG(APP, ERR) << "SDL_SetVideoMode failed\n";
        goto sdl_screen_err;
    }

    m_sdl_inited = true;

sdl_screen_err:
sdl_init_err:
    return;
}

sdl_ui::~sdl_ui()
{
    if (m_sdl_inited) {
        sdl_cleanup();
    }
}

ui_fb* sdl_ui::new_fb(std::string name, const ui_fb_info &info)
{
    if (!m_sdl_inited) {
        sdl_init();
    }

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
            LOG(APP, ERR) << "blit failed: " << SDL_GetError() << "\n";
        }
        SDL_Flip(m_screen);
    }
}
