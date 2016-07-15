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
 * @file manager.h
 * ComponentManager class declaration
 */

#ifndef UTILS_COMPONENT_MANAGER_H
#define UTILS_COMPONENT_MANAGER_H

#include <vector>
#include <map>
#include <string>

#include "rabbits/module/manager.h"

#include "factory.h"

/**
 * @brief Component manager
 *
 * Handles the components collection. Allows for searching a component by type
 * or by name and returns the corresponding component factory.
 */
class ComponentManager : public ModuleManager<ComponentFactoryBase> {
protected:
    Factories m_by_type;

public:
    ComponentManager() {}
    virtual ~ComponentManager() {}

    void register_factory(Factory f)
    {
        ModuleManager<ComponentFactoryBase>::register_factory(f);

        if (type_exists(f->get_type())) {
            LOG(APP, WRN) << "Two components with the same type, priority not yet implemented.\n";
        }

        m_by_type[f->get_type()] = f;
    }

    bool type_exists(const std::string &type) const
    {
        return m_by_type.find(type) != m_by_type.end();
    }

    /**
     * @brief Find a component given its type
     *
     * @param name The component type.
     *
     * @return the component factory associated to the type, NULL if not found.
     */
    Factory find_by_type(const std::string & type)
    {
        if (!type_exists(type)) {
            throw FactoryNotFoundException(type);
        }

        return m_by_type[type];
    }
};

#endif
