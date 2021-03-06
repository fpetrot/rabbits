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

#ifndef _UTILS_RABBITS_EXCEPTION_H
#define _UTILS_RABBITS_EXCEPTION_H

#include <string>
#include <stdexcept>

class RabbitsException : public std::runtime_error {
protected:
    std::string m_what;
    std::string m_backtrace;
    void make_backtrace();

public:
    explicit RabbitsException(const std::string & what)
        : std::runtime_error(what)
        , m_what(what) { make_backtrace(); }
    virtual ~RabbitsException() throw() {}

    const std::string & get_backtrace() { return m_backtrace; }
};

#endif
