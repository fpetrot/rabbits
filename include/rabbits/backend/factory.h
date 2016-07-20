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
 * BackendFactory class declaration
 */

#ifndef UTILS_BACKEND_FACTORY_H
#define UTILS_BACKEND_FACTORY_H

#include <string>
#include <vector>

#include "rabbits/module/factory.h"

class ComponentBase;

/**
 * @brief Base class for Backend factories
 *
 * Note that backend factories are automatically generated from the backend
 * YAML description.
 */
class BackendFactoryBase : public ModuleFactory<ComponentBase> {
protected:
    BackendFactoryBase(ConfigManager &config, const std::string & name,
                         const std::string & description) 
        : ModuleFactory<ComponentBase>(config, name, description, Namespace::get(Namespace::BACKEND))
    {}

public:
    virtual ~BackendFactoryBase() {}

    /**
     * @brief Called during the PlatformBuilder discovery phase.
     *
     * When the PlatformBuilder builds the platform, it begins with a discovery
     * phase where it calls the discovery method of every backend factories
     * involved into the platform building.
     *
     * The factory can gather information about the platform during this phase
     * to be ready when the building phase will effectively happen.
     *
     * @param name The name of the future backend.
     * @param params The parameters of the future backend.
     */
    virtual void discover(const std::string &name, const PlatformDescription &params) {}
};


template <class TBackend>
class BackendFactory : public BackendFactoryBase {
protected:
    virtual TBackend * create(const std::string & name, Parameters & params) 
    {
        TBackend *c = new TBackend(name.c_str(), params, get_config());
        return c;
    }

    BackendFactory(ConfigManager &config, const std::string & name,
                     const std::string & description)
        : BackendFactoryBase(config, name, description)
    {}

public:
    virtual ~BackendFactory() {}
};
#endif

