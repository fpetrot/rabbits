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

#ifndef _RABBITS_PLATFORM_PARSER_IMPL_MODULE_H
#define _RABBITS_PLATFORM_PARSER_IMPL_MODULE_H

#include "../module.h"
#include "../platform.h"
#include "rabbits/config/manager.h"

inline ParserNodeModule::ParserNodeModule(PlatformDescription &d, const std::string n,
                                   ParserNodePlatform &root, const Namespace &ns)
    : ParserNode(d, root), m_name(n), m_ns(ns)
{
    add_field<std::string>("type", m_type);

    ModuleManagerBase &m = get_root().get_config().get_manager_by_namespace(ns);

    if (!m.type_exists(m_type)) {
        throw ModuleTypeNotFoundParseException(d, ns, m_type);
    }

    m_params = m.find_by_type(m_type)->get_params();
    m_params.fill_from_description(d);
}

inline ParserNodeModule::ParserNodeModule(const std::string name, const std::string &type,
                                          const Parameters &params, ParserNodePlatform &root,
                                          const Namespace &ns)
    : ParserNode(root), m_name(name), m_ns(ns), m_type(type), m_params(params)
{
    if (!get_root().get_config().get_manager_by_namespace(ns).type_exists(m_type)) {
        throw ModuleTypeNotFoundParseException(get_descr(), ns, m_type);
    }
}

inline ParserNodeModule::ParserNodeModule(ParserNodePlatform &root, const Namespace &ns)
    : ParserNode(root), m_ns(ns)
{
}

inline ParserNodeModule::~ParserNodeModule() {}

inline const std::string & ParserNodeModule::get_name() const
{
    return m_name;
}

inline const std::string & ParserNodeModule::get_type() const
{
    return m_type;
}

inline const Namespace & ParserNodeModule::get_namespace() const
{
    return m_ns;
}

inline const Parameters & ParserNodeModule::get_params() const
{
    return m_params;
}

template <class T>
inline void ParserNodeModule::add_field(const std::string &name, T& storage)
{
    try {
        ParserNode::add_field<T>(name, storage);
    } catch (MissingFieldParseException e) {
        throw MissingFieldParseException(e.get_node(), m_ns, m_name, name);
    }
}

inline ParserNodeModuleWithPorts::
ParserNodeModuleWithPorts(PlatformDescription &d, const std::string n,
                          ParserNodePlatform &root, const Namespace &ns)
    : ParserNodeModule(d, n, root, ns)
{}

inline ParserNodeModuleWithPorts::
    ParserNodeModuleWithPorts(const std::string &name, const std::string &type,
                              const Parameters &params, ParserNodePlatform &root,
                              const Namespace &ns)
    : ParserNodeModule(name, type, params, root, ns)
{}

inline ParserNodeModuleWithPorts::
ParserNodeModuleWithPorts(HasPortIface *m, ParserNodePlatform &root,
                          const Namespace &ns)
    : ParserNodeModule(root, ns), m_inst(m)
{}

#endif