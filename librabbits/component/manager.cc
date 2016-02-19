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

#include "rabbits/component/manager.h"
#include "rabbits/logger.h"
#include "rabbits/component/factory.h"


ComponentManager* ComponentManager::m_inst = NULL;

ComponentManager::ComponentManager()
{
}

void ComponentManager::register_component(ComponentFactory *f)
{
    if (find_by_name(f->name()) != NULL) {
        WRN_STREAM("Component `" << f->name() << "` already exists. Overwriting\n");
    }

    m_pool.push_back(f);
    m_by_name[f->name()] = m_pool.back();
    m_by_type[f->type()].push_back(m_pool.back());
    DBG_STREAM("Registering component `" << f->name() << "` with type `" << f->type() << "`\n");
}

ComponentFactory* ComponentManager::find_by_name(const std::string & name)
{
    return m_by_name[name];
}

ComponentFactory* ComponentManager::find_by_type(const std::string & name)
{
    std::vector<ComponentFactory*> &v = m_by_type[name];

    if (v.empty()) {
        return NULL;
    }

    return v.front();
}

ComponentManager& ComponentManager::get() {
    if (m_inst == NULL) {
        m_inst = new ComponentManager;
        ComponentFactory::register_components();
    }
    return *m_inst; 
}
