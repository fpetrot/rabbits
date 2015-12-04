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
#include "rabbits/dynloader/dynlib.h"

#if defined(RABBITS_CONFIG_POSIX)
# include "dynlib_posix.h"
# define IMPL DynLibPosix
static const std::string LIB_EXT = "so";
#elif defined(RABBITS_CONFIG_WIN32)
# include "dynlib_win32.h"
# define IMPL DynLibWin32
static const std::string LIB_EXT = "dll";
#endif


const std::string & DynLib::get_lib_extension()
{
    return LIB_EXT;
}

DynLib * DynLib::open(const std::string &fn)
{
    return new IMPL(fn);
}

void DynLib::close(DynLib* l)
{
    delete l;
}
