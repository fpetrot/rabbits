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

#ifndef _RABBITS_MODULE_NAMESPACE_H
#define _RABBITS_MODULE_NAMESPACE_H

#include <string>

class Namespace {
public:
    enum eNamespace {
        GLOBAL, COMPONENT, PLUGIN, BACKEND
    };

private:
    eNamespace m_id;
    std::string m_name;

    Namespace(eNamespace id, const std::string &name) : m_id(id), m_name(name) {}

    static const Namespace namespaces[];

public:
    eNamespace get_id() const {return m_id; }
    std::string get_name() const {return m_name; }

    static const Namespace & get(eNamespace id) { return namespaces[id]; }
};

#endif
