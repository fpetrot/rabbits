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

#ifndef UTILS_COMPONENT_FACTORY_H
#define UTILS_COMPONENT_FACTORY_H

#include <string>
#include <vector>

#include "parameters.h"
#include "manager.h"

class ComponentBase;

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

    virtual std::string name() = 0;
    virtual std::string type() = 0;
    virtual std::string description() = 0;

    virtual void discover(const std::string &name, const PlatformDescription &params) {}
    virtual ComponentBase* create(const std::string &name, const PlatformDescription &params) = 0;

    ComponentParameters get_params() { return m_params; }
};

#endif
