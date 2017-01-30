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

#include "rabbits/datatypes/typeid.h"
#include <systemc>


std::unordered_map<std::type_index, const char * const> typeidentifier::StaticIds::STATIC_IDS {
    { typeid(int), "int" },

    { typeid(bool), "bool" },
    { typeid(std::string), "string" },

    { typeid(uint8_t), "uint8" },
    { typeid(uint16_t), "uint16" },
    { typeid(uint32_t), "uint32" },
    { typeid(uint64_t), "uint64" },

    { typeid(int8_t), "int8" },
    { typeid(int16_t), "int16" },
    { typeid(int32_t), "int32" },
    { typeid(int64_t), "int64" },

    { typeid(double), "float" },
    { typeid(float), "float" },

    { typeid(sc_core::sc_time), "time" },
};
