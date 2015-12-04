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

#ifndef _UTILS_DATATYPES_ADDRESS_RANGE_H
#define _UTILS_DATATYPES_ADDRESS_RANGE_H

#include <inttypes.h>
#include <ostream>

class AddressRange {
protected:
    uint64_t m_begin;
    uint64_t m_size;

public:
    AddressRange() : m_begin(0), m_size(0) {}
    AddressRange(uint64_t begin, uint64_t size) : m_begin(begin), m_size(size) { }

    uint64_t begin() const { return m_begin; }
    uint64_t size()  const { return m_size; }
    uint64_t end()   const { return m_begin + m_size - 1; }
};

inline std::ostream & operator<< (std::ostream &o, const AddressRange &a)
{
    std::ios::fmtflags save = o.flags();
    o.flags(std::ios::hex | std::ios::showbase);
    o << "(" << a.begin() << ":" << a.end() << ")";
    o.flags(save);

    return o;
}

#endif
