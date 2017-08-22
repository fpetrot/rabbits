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

#include "rabbits/utils/pixel/pixel.h"

/* Post-unpack processing */
class FramebufferPostProcessor {
public:
    virtual void fb_post_process(const PixelInfo &src_fmt, const uint8_t* src,
                                 const PixelInfo &dst_fmt, uint8_t* dst,
                                 size_t count) = 0;
};

struct FramebufferInfo {
    enum PixelFormat {
        ARGB_8888 = PixelInfo::get_pixel_format_id(PixelInfo::ARGB, 32, 8, 8, 8, 8),
        RGBA_8888 = PixelInfo::get_pixel_format_id(PixelInfo::RGBA, 32, 8, 8, 8, 8),
        BGRA_8888 = PixelInfo::get_pixel_format_id(PixelInfo::BGRA, 32, 8, 8, 8, 8),

        RGB_888 = PixelInfo::get_pixel_format_id(PixelInfo::ARGB, 24, 8, 8, 8, 0),
        BGR_888 = PixelInfo::get_pixel_format_id(PixelInfo::ABGR, 24, 8, 8, 8, 0),

        RGB_565 = PixelInfo::get_pixel_format_id(PixelInfo::ARGB, 16, 5, 6, 5, 0),
    };

    bool enabled = false;
    bool has_backlight = false;
    int w, h;
    PixelInfo pixel_info;
    FramebufferPostProcessor *post_processor = nullptr;
    void * data;
};
