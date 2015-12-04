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

#ifndef _UI_SDL_FB_H
#define _UI_SDL_FB_H

#include <SDL/SDL.h>

#include "rabbits/ui/ui_fb.h"

class sdl_ui_fb: public ui_fb
{
private:
    void fb_mode_to_sdl_masks(ui_fb_mode_e mode, Uint32 &rmask, Uint32 &gmask,
            Uint32 &bmask, Uint32 &amask, int &byte_per_pix);

protected:
    SDL_Surface * m_fb_surface;

public:
    sdl_ui_fb(const ui_fb_info & info);

    void set_info(const ui_fb_info & info);

    SDL_Surface* get_surface()
    {
        return m_fb_surface;
    }
};

#endif
