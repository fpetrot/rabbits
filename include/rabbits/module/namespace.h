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

#include "rabbits/rabbits_exception.h"


/**
 * @brief Exception raised when a named namespace has not been found.
 */
class NamespaceNotFoundException : public RabbitsException {
protected:
    std::string make_what(std::string comp) { return "Namespace `" + comp + "` not found."; }
public:
    explicit NamespaceNotFoundException(const std::string & comp) : RabbitsException(make_what(comp)) {}
    virtual ~NamespaceNotFoundException() throw() {}
};

class Namespace {
public:
    enum eNamespace {
        GLOBAL, COMPONENT, PLUGIN, BACKEND,
        LAST_NAMESPACE = BACKEND
    };

    static const int COUNT = LAST_NAMESPACE + 1;

private:
    Namespace(const Namespace &);
    Namespace & operator= (const Namespace &);

    eNamespace m_id;
    std::string m_name, m_singular_name;

    Namespace(eNamespace id, const std::string &name, const std::string &singular)
        : m_id(id), m_name(name), m_singular_name(singular) {}

    static const Namespace namespaces[];

public:
    eNamespace get_id() const { return m_id; }
    std::string get_name() const { return m_name; }
    std::string get_singular() const { return m_singular_name; }

    static const Namespace & get(eNamespace id) { return namespaces[id]; }
    static const Namespace & find_by_name(const std::string &name);
};

#endif
