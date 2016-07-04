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

#include <string>

#include "rabbits/plugin/manager.h"
#include "rabbits/plugin/plugin.h"
#include "rabbits/plugin/factory.h"

#include "rabbits/logger.h"

PluginManager* PluginManager::m_inst = NULL;

void PluginManager::register_plugin(PluginFactory *pf)
{
    LOG(APP, DBG) << "Registering plugin `" << pf->get_name() << "`\n";
    m_plugins.push_back(pf->create());
}

PluginManager& PluginManager::get() {
    if (m_inst == NULL) {
        m_inst = new PluginManager;
        PluginFactory::register_plugins();
    }
    return *m_inst;
}
