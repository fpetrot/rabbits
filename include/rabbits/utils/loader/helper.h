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

#include <string>
#include <inttypes.h>

class DebugInitiator;

struct ImageLoadResult {
    enum eResult {
        LOAD_SUCCESS,   /**< Image successfully loaded */
        INCOMPATIBLE,   /**< Image cannot be loaded by this helper */
        LOAD_ERROR,     /**< There were unrecoverable errors during load.
                             Target memory could be in an inconsistent state. */
    };

    eResult result;

    bool has_entry_point = false;
    uint64_t entry_point = 0;

    bool has_load_size = false;
    uint64_t load_size = 0;
};

class ImageLoaderHelper {
public:
    virtual void load_file(const std::string &fn, DebugInitiator &di,
                           uint64_t load_addr, ImageLoadResult &result) = 0;

    virtual void load_data(const void *data, size_t len, DebugInitiator &di,
                           uint64_t load_addr, ImageLoadResult &result) = 0;

    virtual const char * get_name() const = 0;
};
