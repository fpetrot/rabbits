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

#ifndef _DYNLOADER_DYNLIB_H
#define _DYNLOADER_DYNLIB_H

#include "rabbits/rabbits_exception.h"
#include <string>

class DynLib {
public:
    class CannotOpenDynLibException : public RabbitsException {
        std::string build_what(const std::string &fn, const std::string &reason) {
            return "Cannot open " + fn + ": " + reason;
        }
    public:
        CannotOpenDynLibException(const std::string &fn, const std::string &reason)
            : RabbitsException(build_what(fn, reason)) {}
        ~CannotOpenDynLibException() throw () {}
    };

    class SymbolNotFoundException : public RabbitsException {
        std::string build_what(const std::string &sym) {
            return "Symbol not found:" + sym;
        }
    public:
        SymbolNotFoundException(const std::string &sym)
            : RabbitsException(build_what(sym)) {}
        ~SymbolNotFoundException() throw () {}
    };

private:
    /* No copy */
    DynLib(const DynLib&);
    DynLib & operator=(const DynLib&);

protected:
    std::string m_filename;

    DynLib(const std::string &filename) : m_filename(filename) {}

public:
    virtual ~DynLib() {}

    virtual bool check_symbol(const std::string &sym) = 0;
    virtual void * get_symbol(const std::string &sym) = 0;

    static const std::string & get_lib_extension();
    static DynLib * open(const std::string &fn);
    static void close(DynLib *);
};

#endif
