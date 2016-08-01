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

#ifndef _RABBITS_DATATYPES_TYPEID_H
#define _RABBITS_DATATYPES_TYPEID_H

#include <string>
#include <typeindex>
#include <unordered_map>

class TypeId {
public:
    static constexpr const char * const UNKNOWN_TYPE = "?";

private:
    static std::unordered_map<std::type_index, const char * const> m_ids;

public:
    static const char * get_typeid(std::type_index index)
    {
        if (m_ids.find(index) == m_ids.end()) {
            return UNKNOWN_TYPE;
        }

        return m_ids[index];
    }

    template <class T>
    static const char * get_typeid()
    {
        return get_typeid(typeid(T));
    }
};

#endif
