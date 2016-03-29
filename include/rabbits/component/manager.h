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

class ComponentFactory;

/**
 * @brief Component manager
 *
 * Handles the components collection. Allows for searching a component by type
 * or by name and returns the corresponding component factory.
 */
class ComponentManager {
public:
    typedef std::map<std::string, ComponentFactory*>::iterator iterator;
    typedef std::map<std::string, ComponentFactory*>::const_iterator const_iterator;

private:
    static ComponentManager *m_inst;
    ComponentManager();

    /* Non-copyable */
    ComponentManager(const ComponentManager&);
    ComponentManager & operator=(const ComponentManager&);

protected:
    std::vector<ComponentFactory*> m_pool;
    std::map<std::string, ComponentFactory*> m_by_name;
    std::map<std::string, std::vector<ComponentFactory*> > m_by_type;

public:
    virtual ~ComponentManager() {}

    /**
     * @brief Return the singleton instance of the component manager.
     *
     * @return the component manager.
     */
    static ComponentManager & get();

    /**
     * @brief Called by the factories to register themselves.
     *
     * @param ComponentFactory the factory tho register.
     */
    void register_component(ComponentFactory*);

    /**
     * @brief Find a component given its name
     *
     * @param name The component name.
     *
     * @return the component factory associated to the name, NULL if not found.
     */
    ComponentFactory* find_by_name(const std::string & name);

    /**
     * @brief Find a component given its type
     *
     * @param name The component type.
     *
     * @return the component factory associated to the type, NULL if not found.
     */
    ComponentFactory* find_by_type(const std::string & type);

    /**
     * @brief Return an iterator to the first registered component factory.
     *
     * @return an iterator to the first registered component factory.
     */
    iterator begin() { return m_by_name.begin(); }

    /**
     * @brief Return an iterator to the <i>past-the-end</i> registered component factory.
     *
     * @return an iterator to the <i>past-the-end</i> registered component factory.
     */
    iterator end() {  return m_by_name.end(); } 

    /**
     * @brief Return an constant iterator to the first registered component factory.
     *
     * @return an iterator to the first registered component factory.
     */
    const_iterator begin() const { return m_by_name.begin(); }

    /**
     * @brief Return an constant iterator to the <i>past-the-end</i> registered component factory.
     *
     * @return an iterator to the <i>past-the-end</i> registered component factory.
     */
    const_iterator end() const { return m_by_name.end(); }
};

#endif
