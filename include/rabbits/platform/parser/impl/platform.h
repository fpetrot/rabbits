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

#ifndef _RABBITS_PLATFORM_PARSER_IMPL_PLATFORM_H
#define _RABBITS_PLATFORM_PARSER_IMPL_PLATFORM_H

#include "../platform.h"
#include "../component.h"
#include "../backend.h"
#include "../plugin.h"
#include "../module.h"


inline ParserNodePlatform::ParserNodePlatform(PlatformDescription &descr, ConfigManager &config)
    : ParserNode(descr, *this), m_config(config)
{
    add_field<std::string>("description", m_description);
    add_optional_field<bool>("generic", false, m_generic);
    add_optional_field<std::string>("inherit", "", m_inherit);
    add_optional_named_subnodes<ParserNodeComponent>("components", m_components);
    add_optional_named_subnodes<ParserNodeBackend>("backends", m_backends);
    add_optional_named_subnodes<ParserNodePlugin>("plugins", m_plugins);
}

inline const std::string& ParserNodePlatform::get_description() const
{
    return m_description;
}

inline bool ParserNodePlatform::is_generic() const
{
    return m_generic;
}

inline bool ParserNodePlatform::has_parent() const
{
    return m_inherit != "";
}

inline const std::string ParserNodePlatform::get_parent_name() const
{
    return m_inherit;
}


inline ParserNode::NamedSubnodes<ParserNodeComponent> & ParserNodePlatform::get_components()
{
    return m_components;
}

inline ParserNode::NamedSubnodes<ParserNodeBackend> & ParserNodePlatform::get_backends()
{
    return m_backends;
}

inline ParserNode::NamedSubnodes<ParserNodePlugin> & ParserNodePlatform::get_plugins()
{
    return m_plugins;
}

inline bool ParserNodePlatform::module_with_ports_exists(const Namespace &ns,
                                                         const std::string &name) const
{
    switch (ns.get_id()) {
    case Namespace::COMPONENT:
        return m_components.find(name) != m_components.end();

    case Namespace::BACKEND:
        return m_backends.find(name) != m_backends.end();

    default:
        return false;
    }
}

inline std::shared_ptr<ParserNodeModuleWithPorts>
ParserNodePlatform::get_module_with_ports(const Namespace &ns, const std::string &name)
{
    if (!module_with_ports_exists(ns, name)) {
        throw RabbitsException("Module with port `" + ns.get_name() + ":" + name 
                               + "' does not exist");
    }

    switch (ns.get_id()) {
    case Namespace::COMPONENT:
        return m_components[name];
    case Namespace::BACKEND:
        return m_backends[name];
    default:
        std::abort();
    }
}
#endif
