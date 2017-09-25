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

#include <vector>
#include <systemc>

#include "rabbits/datatypes/framebuffer.h"
#include "rabbits/utils/pixel/pixel.h"

class FramebufferSystemCInterface : public virtual sc_core::sc_interface {
public:
    virtual void set_info(const FramebufferInfo &) = 0;
    virtual void set_palette(const std::vector<uint32_t> &palette) = 0;
    virtual void set_backlight_level(uint8_t lvl) = 0;
};