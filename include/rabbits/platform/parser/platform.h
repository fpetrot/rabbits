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

#ifndef _RABBITS_PLATFORM_PARSER_PLATFORM_H
#define _RABBITS_PLATFORM_PARSER_PLATFORM_H

#include "node.h"
#include "rabbits/config/has_config.h"

class ParserNodeModule;
class ParserNodeComponent;
class ParserNodeBackend;
class ParserNodePlugin;
class ParserNodeModuleWithPorts;

class ComponentBase;
class PluginBase;
class Parameters;

class ParserNodePlatform : public ParserNode, public HasConfigIface {
protected:
    std::string m_description;
    bool m_generic = false;
    std::string m_inherit;
    ConfigManager &m_config;

    NamedSubnodes<ParserNodeComponent> m_components;
    NamedSubnodes<ParserNodeBackend> m_backends;
    NamedSubnodes<ParserNodePlugin> m_plugins;

public:
    ParserNodePlatform(PlatformDescription &descr, ConfigManager &config);
    ParserNodePlatform(ConfigManager &config);

    const std::string& get_description() const;
    bool is_generic() const;
    bool has_parent() const;
    const std::string get_parent_name() const;

    NamedSubnodes<ParserNodeComponent> & get_components();
    NamedSubnodes<ParserNodeBackend> & get_backends();
    NamedSubnodes<ParserNodePlugin> & get_plugins();

    bool module_exists(const Namespace &ns, const std::string &name) const;
    std::shared_ptr<ParserNodeModule> get_module(const Namespace &ns, const std::string &name);

    bool module_with_ports_exists(const Namespace &ns, const std::string &name) const;
    std::shared_ptr<ParserNodeModuleWithPorts> get_module_with_ports(const Namespace &ns, const std::string &name);

    bool component_exists(const std::string & name) const;
    std::shared_ptr<ParserNodeComponent> get_component(const std::string & name);
    bool backend_exists(const std::string & name) const;
    std::shared_ptr<ParserNodeBackend> get_backend(const std::string & name);
    bool plugin_exists(const std::string & name) const;
    std::shared_ptr<ParserNodePlugin> get_plugin(const std::string & name);

    void find_component_by_attr(const std::string &key, Subnodes<ParserNodeComponent> &out);
    void find_backend_by_attr(const std::string &key, Subnodes<ParserNodeBackend> &out);

    ConfigManager & get_config() const { return m_config; }

    std::shared_ptr<ParserNodeComponent> create_component(const std::string name, const std::string type,
                                                          const Parameters &params);

    std::shared_ptr<ParserNodeBackend> create_backend(const std::string name, const std::string type,
                                                      const Parameters &params);

    std::shared_ptr<ParserNodePlugin> create_plugin(const std::string name, const std::string type,
                                                     const Parameters &params);

    void add_component(ComponentBase *c);
    void add_backend(ComponentBase *c);
    void add_plugin(PluginBase *c);

    bool empty() const;
};

#endif
