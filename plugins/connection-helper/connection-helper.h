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

#pragma once

#include <rabbits/plugin/plugin.h>
#include <rabbits/platform/parser.h>

class Port;
class ComponentBase;

class ConnectionHelperPlugin : public Plugin {
private:
    bool m_stdio_locked = false;
    int m_unique_idx = 0;

    ParserNode::Subnodes<ParserNodeComponent> m_char_nodes;

protected:
    bool stdio_is_locked(PlatformParser &p);

    std::string get_param(const std::string comp) const;
    std::string gen_unique_name(const std::string & type);

    void create_params(const PluginHookAfterComponentInst &);
    void parse_params(const PluginHookAfterComponentInst &);

    void autoconnect(const PluginHookAfterBuild &h, ParserNodeBackend &);
    void autoconnect(const PluginHookAfterBuild &h, ParserNodeComponent &);
    void autoconnect(const PluginHookAfterBuild &h, ComponentBase &, bool is_comp);
    void autoconnect(const PluginHookAfterBuild &h, Port &, bool is_comp);

public:
    ConnectionHelperPlugin(const std::string &name,
                                  const Parameters &params,
                                  ConfigManager &config)
        : Plugin(name, params, config)
    {}

    virtual ~ConnectionHelperPlugin() {}

    void hook(const PluginHookAfterComponentInst &);
    void hook(const PluginHookAfterBuild &);
};
