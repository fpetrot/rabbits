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
#include "../binding.h"
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
}

inline ParserNodeModule::ParserNodeModule(const std::string name, const std::string &type,
                                          const Parameters &params, ParserNodePlatform &root,
                                          const Namespace &ns)
    : ParserNode(root), m_name(name), m_ns(ns), m_type(type)
{
    set_params(params);
    if (!get_root().get_config().get_manager_by_namespace(ns).type_exists(m_type)) {
        throw ModuleTypeNotFoundParseException(get_descr(), ns, m_type);
    }
}

inline ParserNodeModule::ParserNodeModule(ParserNodePlatform &root,
                                          const Namespace &ns)
    : ParserNode(root), m_ns(ns)
{
}

inline ParserNodeModule::~ParserNodeModule() {}

inline ModuleManagerBase::Factory ParserNodeModule::get_module_factory()
{
    ModuleManagerBase &m = get_root().get_config().get_manager_by_namespace(m_ns);
    return m.find_by_type(m_type);
}

inline void ParserNodeModule::set_params(const Parameters &p)
{
    m_params = p;
    m_params_is_set = true;
}

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

inline ModuleManagerBase::Factory ParserNodeModule::get_factory()
{
    if (m_factory == nullptr) {
        m_factory = get_module_factory();
    }

    return m_factory;
}

inline const Parameters & ParserNodeModule::get_params()
{
    if (!m_params_is_set) {
        ModuleManagerBase::Factory f = get_factory();

        set_params(f->get_params());
        m_params.fill_from_description(get_descr());
    }

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
{
    add_optional_named_subnodes<ParserNodeBinding>("bindings", m_bindings, *this);
}

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

inline bool ParserNodeModuleWithPorts::binding_exists(const std::string &port) const
{
    return m_bindings.find(port) != m_bindings.end();
}

inline void ParserNodeModuleWithPorts::add_binding(const std::string local_port,
                                             std::shared_ptr<ParserNodeModuleWithPorts> peer,
                                             const std::string &peer_port,
                                             PlatformDescription &params)
{
    if (binding_exists(local_port)) {
        throw RabbitsException("Binding already exists for port " + local_port + " of component " + get_name());
    }

    auto binding = std::make_shared<ParserNodeBinding>(local_port, peer, peer_port,
                                                       params, get_root(), *this);

    m_bindings[local_port] = binding;
    add_subnode(binding);
}

inline void ParserNodeModuleWithPorts::remove_binding_if_exists(const std::string local_port)
{
    if (binding_exists(local_port)) {
        remove_subnode(m_bindings[local_port]);
        m_bindings.erase(local_port);
    }
}

#endif
