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

#ifndef _RABBITS_PLATFORM_PARSER_IMPL_PLUGIN_H
#define _RABBITS_PLATFORM_PARSER_IMPL_PLUGIN_H

#include "../plugin.h"
#include "../platform.h"

inline ParserNodePlugin::ParserNodePlugin(PlatformDescription &d, const std::string &n, ParserNodePlatform &root)
    : ParserNodeModule(d, n, root, Namespace::get(Namespace::PLUGIN))
{}

inline ParserNodePlugin::ParserNodePlugin(const std::string name, const std::string &type,
                                          const Parameters &params, ParserNodePlatform &root)
    : ParserNodeModule(name, type, params, root, Namespace::get(Namespace::PLUGIN))
{}

inline ParserNodePlugin::ParserNodePlugin(PluginBase *p, ParserNodePlatform &root)
    : ParserNodeModule(PlatformDescription::INVALID_DESCRIPTION, p->get_name(),
                       root, Namespace::get(Namespace::PLUGIN))
{
    set_inst(p);
}

inline ParserNodePlugin::~ParserNodePlugin()
{}

inline PluginManager::Factory ParserNodePlugin::get_plugin_factory()
{
    PluginManager &pm = get_root().get_config().get_plugin_manager();
    return pm.find_by_type(get_type());
}

#endif
