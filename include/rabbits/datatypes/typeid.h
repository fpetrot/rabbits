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
#include <vector>

namespace typeidentifier {

struct StaticIds {
    static std::unordered_map<std::type_index, const char * const> STATIC_IDS;
};

template <class T>
struct TypeIdentifier {
    static constexpr const char * const UNKNOWN_TYPE = "?";


    static const char * get_typeid() {
        if (StaticIds::STATIC_IDS.find(typeid(T)) == StaticIds::STATIC_IDS.end()) {
            return UNKNOWN_TYPE;
        }

        return StaticIds::STATIC_IDS[typeid(T)];
    }
};

template <class T>
struct TypeIdentifier<std::vector<T> > {
private:
    static std::string m_id;
public:
    static const char * get_typeid() {
        if (m_id.empty()) {
            m_id = "vector(" + std::string(TypeIdentifier<T>::get_typeid()) + ")";
        }

        return m_id.c_str();
    }
};

}

class TypeId {
public:
    template <class T>
    static const char * get_typeid()
    {
        return typeidentifier::TypeIdentifier<T>::get_typeid();
    }
};

template <class T>
std::string typeidentifier::TypeIdentifier< std::vector<T> >::m_id;

#endif
