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

#ifndef _RABBITS_PLATFORM_PARSER_IMPL_COMPONENT_H
#define _RABBITS_PLATFORM_PARSER_IMPL_COMPONENT_H

#include "../component.h"
#include "../binding.h"
#include "../platform.h"

#include "rabbits/component/component.h"

inline ParserNodeComponent::ParserNodeComponent(PlatformDescription &descr, const std::string &n,
                                         ParserNodePlatform &root)
    : ParserNodeModuleWithPorts(descr, n, root, Namespace::get(Namespace::COMPONENT))
{
    add_optional_field<std::string>("implementation", "", m_implem);
    add_optional_named_subnodes<ParserNodeBinding>("bindings", m_bindings, *this);
}

inline ParserNodeComponent::ParserNodeComponent(const std::string &name, const std::string &type,
                                                const Parameters &params, ParserNodePlatform &root)
    : ParserNodeModuleWithPorts(name, type, params, root, Namespace::get(Namespace::COMPONENT))
{
}

inline ParserNodeComponent::ParserNodeComponent(ComponentBase *c, ParserNodePlatform &root)
    : ParserNodeModuleWithPorts(c, root, Namespace::get(Namespace::COMPONENT)), m_inst(c)
{}

inline ParserNodeComponent::~ParserNodeComponent() {}

inline bool ParserNodeComponent::implem_is_set() const { return !m_implem.empty(); }

inline const std::string & ParserNodeComponent::get_implem() const { return m_implem; }

inline void ParserNodeComponent::set_inst(ComponentBase *inst)
{
    m_inst = inst;
    ParserNodeModuleWithPorts::set_inst(inst);
}

inline bool ParserNodeComponent::binding_exists(const std::string &port) const
{
    return m_bindings.find(port) != m_bindings.end();
}

inline void ParserNodeComponent::add_binding(const std::string local_port,
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

inline void ParserNodeComponent::remove_binding_if_exists(const std::string local_port)
{
    if (binding_exists(local_port)) {
        remove_subnode(m_bindings[local_port]);
        m_bindings.erase(local_port);
    }
}

#endif
