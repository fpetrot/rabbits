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

#ifndef _RABBITS_PLATFORM_PARSER_EXCEPTIONS_H
#define _RABBITS_PLATFORM_PARSER_EXCEPTIONS_H

#include "rabbits/rabbits_exception.h"
#include "rabbits/platform/description.h"
#include "rabbits/module/namespace.h"
#include "rabbits/module/module.h"


class PlatformParseException : public RabbitsException {
private:
    std::string make_what(const PlatformDescription &node) const {
        return "Parsing exception" + node.origin();
    }

protected:
    const PlatformDescription m_node;

    std::string get_origin(const PlatformDescription &node) const
    {
        return " at " + node.origin();
    }

    std::string get_module(const Namespace &ns, const std::string name) const
    {
        return " for " + ns.get_singular() + " `" + name + "`";
    }

public:
    PlatformParseException(const PlatformDescription &node)
        : RabbitsException(make_what(node)), m_node(node) {}

    PlatformParseException(const PlatformDescription &node,
                           const std::string & what)
        : RabbitsException(what), m_node(node) {}

    virtual ~PlatformParseException() throw() {}

    const PlatformDescription & get_node() const { return m_node; }
};

class MissingFieldParseException : public PlatformParseException {
protected:
    const std::string m_field;

    std::string make_what(const std::string &field) const
    {
        return "Missing field `" + field + "`";
    }

    std::string make_what(const PlatformDescription &node,
                          const std::string &field) const
    {
        return make_what(field) + get_origin(node);
    }

    std::string make_what(const PlatformDescription &node,
                          const Namespace &ns,
                          const std::string &mod,
                          const std::string &field) const
    {
        return make_what(field) + get_module(ns, mod) + get_origin(node);
    }

public:
    MissingFieldParseException(const PlatformDescription &node,
                               const std::string & field)
        : PlatformParseException(node, make_what(node, field)), m_field(field)
    {}

    MissingFieldParseException(const PlatformDescription &node,
                               const Namespace &ns,
                               const std::string &mod,
                               const std::string & field)
        : PlatformParseException(node, make_what(node, ns, mod, field)), m_field(field)
    {}

    virtual ~MissingFieldParseException() throw() {}

    const std::string & get_field() const { return m_field; }
};

class InvalidFieldTypeException : public PlatformParseException {
protected:
    const std::string m_field;
    const char * m_type_id;

    std::string make_what(const PlatformDescription &node,
                          const std::string &f, const char *t) const {
        return "Invalid type for field `" + f
            + "`. Expected `" + t + "`" + get_origin(node);
    }

public:
    InvalidFieldTypeException(const PlatformDescription &node,
                              const std::string & field, const char * type_id)
        : PlatformParseException(node, make_what(node, field, type_id))
        , m_field(field), m_type_id(type_id)
    {}

    virtual ~InvalidFieldTypeException() throw() {}
};


class NamespaceNotFoundParseException: public PlatformParseException {
protected:
    const std::string m_ns;

    std::string make_what(const PlatformDescription &node,
                          const std::string &ns)
    {
        return "Namespace `" + ns + "` not found" + get_origin(node);
    }

public:
    NamespaceNotFoundParseException(const PlatformDescription &node,
                                    const std::string &ns)
        : PlatformParseException(node, make_what(node, ns))
        , m_ns(ns)
    {}

    virtual ~NamespaceNotFoundParseException() throw() {}

};

class ModuleTypeNotFoundParseException : public PlatformParseException {
protected:
    std::string make_what(const PlatformDescription &node,
                          const Namespace &ns, const std::string &mod)
    {
        return "Module type `" + ns.get_name() + "." + mod
            + "` not found" + get_origin(node);
    }

public:
    ModuleTypeNotFoundParseException(const PlatformDescription &node,
                                     const Namespace &ns, const std::string &mod)
        : PlatformParseException(node, make_what(node, ns, mod))
    {}

    virtual ~ModuleTypeNotFoundParseException() throw() {}
};

class ComponentImplemNotFoundParseException : public PlatformParseException {
protected:
    std::string make_what(const PlatformDescription &node,
                          const Namespace &ns, const std::string &mod)
    {
        return "Component implementation `" + mod
            + "` not found" + get_origin(node);
    }

public:
    ComponentImplemNotFoundParseException(const PlatformDescription &node,
                                     const Namespace &ns, const std::string &mod)
        : PlatformParseException(node, make_what(node, ns, mod))
    {}

    virtual ~ComponentImplemNotFoundParseException() throw() {}
};

class ModuleNotFoundParseException : public PlatformParseException {
protected:
    const Namespace &m_ns;
    const std::string m_mod;

    std::string make_what(const PlatformDescription &node,
                          const Namespace &ns, const std::string &mod)
    {
        return "Module `" + ns.get_name() + ":" + mod
            + "` not found" + get_origin(node);
    }

public:
    ModuleNotFoundParseException(const PlatformDescription &node,
                                 const Namespace &ns, const std::string &mod)
        : PlatformParseException(node, make_what(node, ns, mod))
        , m_ns(ns), m_mod(mod)
    {}

    virtual ~ModuleNotFoundParseException() throw() {}
};

class NoPortFoundParseException : public PlatformParseException {
protected:
    std::string make_what(const PlatformDescription &node,
                          const Namespace &ns, const std::string &mod)
    {
        return "No port found on module `"
            + ns.get_name() + ":" + mod + "`" + get_origin(node);
    }

public:
    NoPortFoundParseException(const PlatformDescription &node,
                              const Namespace &ns, const std::string &mod)
        : PlatformParseException(node, make_what(node, ns, mod))
    {}
};

class PortNotFoundParseException : public PlatformParseException {
protected:
    std::string make_what(const PlatformDescription &node,
                          const Namespace &ns, const std::string &mod,
                          const std::string port)
    {
        return "Port `" + port + "` not found on module `"
            + ns.get_name() + ":" + mod + "`" + get_origin(node);
    }

public:
    PortNotFoundParseException(const PlatformDescription &node,
                               const Namespace &ns, const std::string &mod,
                               const std::string port)
        : PlatformParseException(node, make_what(node, ns, mod, port))
    {}

    virtual ~PortNotFoundParseException() throw() {}
};

#endif
