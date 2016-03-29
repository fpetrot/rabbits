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

#include "parameters.h"
#include "manager.h"

class ComponentBase;

/**
 * @brief Base class for Component factories
 *
 * Note that component factories are automatically generated from the component
 * YAML description.
 */
class ComponentFactory {
private:
    ComponentParameters m_params;

    ComponentFactory(const ComponentFactory&);
    ComponentFactory & operator=(const ComponentFactory&);

    static std::vector<ComponentFactory *> *m_insts;

protected:
    ComponentFactory() {
        if (m_insts == NULL) {
            m_insts = new std::vector<ComponentFactory*>;
        }
        m_insts->push_back(this);
    }

    template <typename T>
    void add_param(const std::string &name, const T &p) {
        m_params.add(name, p);
    }


public:
    /**
     * @brief Called by the ComponentManager to register the components at runtime.
     */
    static void register_components() {
        std::vector<ComponentFactory*>::iterator it;

        if (m_insts == NULL) {
            return;
        }

        for (it = m_insts->begin(); it != m_insts->end(); it++) {
            ComponentManager::get().register_component(*it);
        }
    }

    virtual ~ComponentFactory() {}

    /**
     * @brief Return the name of the component.
     *
     * Return the name of the component associated to this factory.
     *
     * @return the name of the component.
     */
    virtual std::string name() = 0;

    /**
     * @brief Return the type of the component.
     *
     * Return the type of the component associated to this factory.
     *
     * @return the type of the component.
     */
    virtual std::string type() = 0;

    /**
     * @brief Return the description of the component.
     *
     * Return the description of the component associated to this factory.
     *
     * @return the description of the component.
     */
    virtual std::string description() = 0;

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

    /**
     * @brief Creates a component associated to this factory and returns it.
     *
     * @param name The component name.
     * @param params The component parameter.
     *
     * @return The newly created component.
     */
    virtual ComponentBase* create(const std::string &name, const PlatformDescription &params) = 0;

    /**
     * @brief Return the parameters of the components associated to this factory.
     *
     * This method allows to discover the parameters of a component associated
     * to this factory, and their default values.
     *
     * @return the components parameters and default values.
     */
    ComponentParameters get_params() { return m_params; }
};

#endif
