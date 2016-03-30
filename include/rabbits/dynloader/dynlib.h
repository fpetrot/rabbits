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

/**
 * @file dynlib.h
 * @brief DynLib class declaration
 */

#ifndef _DYNLOADER_DYNLIB_H
#define _DYNLOADER_DYNLIB_H

#include "rabbits/rabbits_exception.h"
#include <string>

/**
 * @brief A dynamic library
 */
class DynLib {
public:
    /**
     * @brief Raised when a dynamic library cannot be open.
     */
    class CannotOpenDynLibException : public RabbitsException {
        std::string build_what(const std::string &fn, const std::string &reason) {
            return "Cannot open " + fn + ": " + reason;
        }
    public:
        CannotOpenDynLibException(const std::string &fn, const std::string &reason)
            : RabbitsException(build_what(fn, reason)) {}
        ~CannotOpenDynLibException() throw () {}
    };

    /**
     * @brief Raised when the requested symbol is not found in a dynamic library.
     */
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

    /**
     * @brief Check if the given symbol exists.
     *
     * @param[in] sym The symbol name.
     *
     * @return true if the symbol exists in the dynamic library, false otherwise.
     */
    virtual bool check_symbol(const std::string &sym) = 0;

    /**
     * @brief Return the address of the given symbol.
     *
     * @param[in] sym The symbol name.
     *
     * @return the address of the given symbol.
     * @throw SymbolNotFoundException if the symbol does not exists.
     */
    virtual void * get_symbol(const std::string &sym) = 0;

    /**
     * @brief Return the file extension of dynamic libraries on the current operating system.
     *
     * @return the file extension of dynamic libraries on the current operating system.
     */
    static const std::string & get_lib_extension();

    /**
     * @brief Open a dynamic library.
     *
     * This method is meant to be used by the DynamicLoader, and not directly.
     * If you want to load a dynamic library, see the DynamicLoader class.
     *
     * @param[in] fn path to the dynamic library.
     *
     * @return the dynamic library
     * @throw CannotOpenDynLibException if the operation fails.
     *
     * @see DynamicLoader
     */
    static DynLib * open(const std::string &fn);

    /**
     * @brief Close a dynamic library.
     *
     * This method is meant to be used by the DynamicLoader, and not directly.
     *
     * @param[in] lib The dynamic library
     *
     * @see DynamicLoader
     */
    static void close(DynLib *lib);
};

#endif
