/*
 *  This file is part of Rabbits
 *  Copyright (C) 2015-2017  Clement Deschamps and Luc Michel
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

/**
 * @file loader.h
 * @brief Loader class declaration
 */

#pragma once

#include <string>
#include <list>

#include "helper.h"

class DebugInitiator;

class ImageLoader {
protected:
    std::list<ImageLoaderHelper*> m_helpers;

public:
    ImageLoader();
    virtual ~ImageLoader() {}

    void load_file(const std::string &fn, DebugInitiator &di,
                   uint64_t load_addr, ImageLoadResult &result);

    void load_data(const void *data, size_t len, DebugInitiator &di,
                   uint64_t load_addr, ImageLoadResult &result);

    void register_helper(ImageLoaderHelper *helper);
};
