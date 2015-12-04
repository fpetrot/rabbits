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

#include "ui_fb.h"

#include "rabbits-common.h"
#include "rabbits/logger.h"

sdl_ui_fb::sdl_ui_fb(const ui_fb_info & info) :
        ui_fb(info)
{
    m_fb_surface = NULL;

    set_info(info);
}

void sdl_ui_fb::fb_mode_to_sdl_masks(ui_fb_mode_e mode, Uint32 &rmask,
        Uint32 &gmask, Uint32 &bmask, Uint32 &amask, int &byte_per_pix)
{
    switch (mode) {
    case FB_MODE_RGB888:
        rmask = 0x00ff0000;
        gmask = 0x0000ff00;
        bmask = 0x000000ff;
        amask = 0x00000000;
        byte_per_pix = 3;
        break;

    case FB_MODE_RGB565:
        rmask = 0x0000f800;
        gmask = 0x000007e0;
        bmask = 0x0000001f;
        amask = 0x00000000;
        byte_per_pix = 2;
        break;
    }
}

void sdl_ui_fb::set_info(const ui_fb_info & info)
{
    m_info = info;

    /* XXX */
    if (m_fb_surface != NULL) {
        return;
    }

    if (m_info.buf != NULL) {
        Uint32 rmask, gmask, bmask, amask;
        int byte_per_pix;

        fb_mode_to_sdl_masks(m_info.mode, rmask, gmask, bmask, amask,
                byte_per_pix);

        /* If pitch is not set, assume line length */
        if (!m_info.pitch) {
            m_info.pitch = m_info.virtual_w * byte_per_pix;
        }

        m_fb_surface = SDL_CreateRGBSurfaceFrom(m_info.buf, m_info.virtual_w,
                m_info.virtual_h, byte_per_pix << 3, m_info.pitch, rmask, gmask,
                bmask, amask);

        if (m_fb_surface == NULL) {
            ERR_PRINTF("Unable to create fb SDL surface\n");
            goto create_surf_err;
        }

        DBG_PRINTF("Created SDL surface: bpp:%u, (%d,%d)\n",
                m_fb_surface->format->BitsPerPixel,
                m_fb_surface->w, m_fb_surface->h);

    }

    return;

create_surf_err:
    return;
}

