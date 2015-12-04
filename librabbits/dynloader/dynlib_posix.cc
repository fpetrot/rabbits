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

#include "dynlib_posix.h"

#include <dlfcn.h>

DynLibPosix::DynLibPosix(const std::string &filename)
    : DynLib(filename)
{
    m_handle = dlopen(filename.c_str(), RTLD_NOW);

    if (m_handle == NULL) {
        throw CannotOpenDynLibException(filename, dlerror());
    }
}

DynLibPosix::~DynLibPosix()
{
    if (m_handle != NULL) {
        dlclose(m_handle);
    }
}

bool DynLibPosix::check_symbol(const std::string &sym)
{
    dlerror();  /* Clear error if any */

    if ((dlsym(m_handle, sym.c_str()) == NULL) && (dlerror() != NULL)) {
        return false;
    }

    return true;
}

void * DynLibPosix::get_symbol(const std::string &sym)
{
    void* sym_addr;

    dlerror();

    sym_addr = dlsym(m_handle, sym.c_str());

    if ((sym_addr == NULL) && (dlerror() != NULL)) {
        throw SymbolNotFoundException(sym);
    }

    return sym_addr;
}
