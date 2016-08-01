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
 * Handles the components collection. Allows for searching a component by implementation
 * or by type and returns the corresponding component factory.
 */
class ComponentManager : public ModuleManager<ComponentFactoryBase> {
protected:
    Factories m_by_implem;

public:
    ComponentManager() {}
    virtual ~ComponentManager() {}

    void register_factory(Factory f)
    {
        if (type_exists(f->get_type())) {
            LOG(APP, WRN) << "Two components with the same type. Priority not yet implemented.\n";
        }

        ModuleManager<ComponentFactoryBase>::register_factory(f);

        if (implem_exists(f->get_implem())) {
            LOG(APP, WRN) << "Two components with the same implementation name. Overwritting.\n";
        }

        m_by_implem[f->get_implem()] = f;
    }

    bool implem_exists(const std::string &implem) const
    {
        return m_by_implem.find(implem) != m_by_implem.end();
    }

    /**
     * @brief Find a component given its implem
     *
     * @param name The component implem.
     *
     * @return the component factory associated to the implem, NULL if not found.
     */
    Factory find_by_implem(const std::string & implem)
    {
        if (!implem_exists(implem)) {
            throw FactoryNotFoundException(implem);
        }

        return m_by_implem[implem];
    }
};

#endif
