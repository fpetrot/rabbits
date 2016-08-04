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

#include "rabbits/component/component.h"
#include "rabbits/plugin/plugin.h"

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

inline ParserNodePlatform::ParserNodePlatform(ConfigManager &config)
    : ParserNode(*this), m_config(config)
{}

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

inline bool ParserNodePlatform::
module_exists(const Namespace &ns, const std::string &name) const
{
    switch (ns.get_id()) {
    case Namespace::COMPONENT:
    case Namespace::BACKEND:
        return module_with_ports_exists(ns, name);

    case Namespace::PLUGIN:
        return m_plugins.find(name) != m_plugins.end();

    default:
        return false;
    }
}

inline std::shared_ptr<ParserNodeModule>
ParserNodePlatform::get_module(const Namespace &ns, const std::string &name)
{
    if (!module_exists(ns, name)) {
        throw RabbitsException("Module `" + ns.get_name() + ":" + name
                               + "' does not exist");
    }

    switch (ns.get_id()) {
    case Namespace::COMPONENT:
    case Namespace::BACKEND:
        return get_module_with_ports(ns, name);

    case Namespace::PLUGIN:
        return m_plugins[name];

    default:
        std::abort();
    }
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

inline bool ParserNodePlatform::component_exists(const std::string & name) const
{
    return module_exists(Namespace::get(Namespace::COMPONENT), name);
}

inline std::shared_ptr<ParserNodeComponent>
ParserNodePlatform::get_component(const std::string & name)
{
    return std::shared_ptr<ParserNodeComponent>
        (static_cast<ParserNodeComponent*>
         (get_module(Namespace::get(Namespace::COMPONENT), name).get()));
}

inline bool ParserNodePlatform::backend_exists(const std::string & name) const
{
    return module_exists(Namespace::get(Namespace::BACKEND), name);
}

inline std::shared_ptr<ParserNodeBackend>
ParserNodePlatform::get_backend(const std::string & name)
{
    return std::shared_ptr<ParserNodeBackend>
        (static_cast<ParserNodeBackend*>
         (get_module(Namespace::get(Namespace::BACKEND), name).get()));
}

inline bool ParserNodePlatform::plugin_exists(const std::string & name) const
{
    return module_exists(Namespace::get(Namespace::PLUGIN), name);
}

inline std::shared_ptr<ParserNodePlugin>
ParserNodePlatform::get_plugin(const std::string & name)
{
    return std::shared_ptr<ParserNodePlugin>
        (static_cast<ParserNodePlugin*>
         (get_module(Namespace::get(Namespace::PLUGIN), name).get()));
}

inline void ParserNodePlatform::find_component_by_attr(const std::string &key, Subnodes<ParserNodeComponent> &out)
{
    for (auto &c : get_components()) {
        ComponentBase *comp = c.second->get_inst();
        if ((comp != nullptr) && (comp->has_attr(key))) {
            out.push_back(c.second);
        }
    }
}

inline void ParserNodePlatform::find_backend_by_attr(const std::string &key, Subnodes<ParserNodeBackend> &out)
{
    for (auto &c : get_backends()) {
        ComponentBase *comp = c.second->get_inst();
        if ((comp != nullptr) && (comp->has_attr(key))) {
            out.push_back(c.second);
        }
    }
}

inline std::shared_ptr<ParserNodeComponent>
ParserNodePlatform::create_component(const std::string name, const std::string type,
                                     const Parameters &params)
{
    if (component_exists(name)) {
        throw RabbitsException(std::string("Component `") + name
                               + "` already exists.");
    }

    m_components[name] = std::make_shared<ParserNodeComponent>(name, type,
                                                               params, *this);
    add_subnode(m_components[name]);

    return m_components[name];
}

inline std::shared_ptr<ParserNodeBackend>
ParserNodePlatform::create_backend(const std::string name, const std::string type,
                                   const Parameters &params)
{
    if (backend_exists(name)) {
        throw RabbitsException(std::string("Backend `") + name
                               + "` already exists.");
    }

    m_backends[name] = std::make_shared<ParserNodeBackend>(name, type,
                                                           params, *this);
    add_subnode(m_backends[name]);

    return m_backends[name];
}

inline std::shared_ptr<ParserNodePlugin>
ParserNodePlatform::create_plugin(const std::string name, const std::string type,
                                  const Parameters &params)
{
    if (plugin_exists(name)) {
        throw RabbitsException(std::string("Plugin `") + name + "` already exists.");
    }

    m_plugins[name] = std::make_shared<ParserNodePlugin>(name, type, params, *this);
    add_subnode(m_plugins[name]);

    return m_plugins[name];
}


inline void ParserNodePlatform::add_component(ComponentBase *c)
{
    m_components[c->get_name()] = std::make_shared<ParserNodeComponent>(c, *this);
}

inline void ParserNodePlatform::add_backend(ComponentBase *c)
{
    m_backends[c->get_name()] = std::make_shared<ParserNodeBackend>(c, *this);
}

inline void ParserNodePlatform::add_plugin(PluginBase *p)
{
    m_plugins[p->get_name()] = std::make_shared<ParserNodePlugin>(p, *this);
}

#endif
