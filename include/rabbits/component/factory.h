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
 * ComponentFactory class declaration
 */

#ifndef UTILS_COMPONENT_FACTORY_H
#define UTILS_COMPONENT_FACTORY_H

#include <string>
#include <vector>

#include "rabbits/module/factory.h"

class ComponentBase;

/**
 * @brief Base class for Component factories
 *
 * Note that component factories are automatically generated from the component
 * YAML description.
 */
class ComponentFactoryBase : public ModuleFactory<ComponentBase> {
private:
    std::string m_implem;

protected:
    ComponentFactoryBase(ConfigManager &config, const std::string & type,
                         const std::string & description, const std::string & implem)
        : ModuleFactory<ComponentBase>(config, type, description, Namespace::get(Namespace::COMPONENT))
        , m_implem(implem)
    {}

public:
    virtual ~ComponentFactoryBase() {}

    /**
     * @brief Return the implementation name of the component factory.
     *
     * Return the implementation name of the component factory.
     *
     * @return the implementation name of the component factory.
     */
    std::string get_implem() const { return m_implem; }

    virtual void get_extra_values(ExtraValues &v) const
    {
        v.push_back(std::make_pair("implementation", get_implem()));
    }

    /**
     * @brief Called during the PlatformBuilder discovery phase.
     *
     * When the PlatformBuilder builds the platform, it begins with a discovery
     * phase where it calls the discovery method of every component factories
     * involved into the platform building.
     *
     * The factory can gather information about the platform during this phase
     * to be ready when the building phase will effectively happen.
     *
     * @param name The name of the future component.
     * @param params The parameters of the future component.
     */
    virtual void discover(const std::string &name, const PlatformDescription &params) {}
};


template <class TComponent>
class ComponentFactory : public ComponentFactoryBase {
protected:
    virtual TComponent * create(const std::string & name, Parameters & params)
    {
        TComponent *c = new TComponent(name.c_str(), params, get_config());
        return c;
    }

    ComponentFactory(ConfigManager &config, const std::string & name,
                     const std::string & description, const std::string & type)
        : ComponentFactoryBase(config, name, description, type)
    {}

public:
    virtual ~ComponentFactory() {}
};
#endif
