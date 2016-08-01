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

/**
 * @file factory.h
 * @brief PluginFactory class declaration.
 */

#ifndef _UTILS_PLUGIN_FACTORY_H
#define _UTILS_PLUGIN_FACTORY_H

#include <string>
#include <vector>

#include "rabbits/module/factory.h"

class PluginBase;

/**
 * @brief Plugin factory base class.
 *
 * Note that plugin factories are automatically generated from the plugin
 * YAML description.
 */
class PluginFactoryBase : public ModuleFactory<PluginBase> {
protected:
    PluginFactoryBase(ConfigManager &config, const std::string & type, const std::string & description)
        : ModuleFactory<PluginBase>(config, type, description, Namespace::get(Namespace::PLUGIN))
    {}

public:
    virtual ~PluginFactoryBase() {}
};

template <class TPlugin>
class PluginFactory : public PluginFactoryBase {
protected:
    virtual TPlugin * create(const std::string & name, Parameters & params)
    {
        return new TPlugin(name, params, get_config());
    }

    PluginFactory(ConfigManager &config, const std::string name, const std::string description)
        : PluginFactoryBase(config, name, description)
    {}
};

#endif
