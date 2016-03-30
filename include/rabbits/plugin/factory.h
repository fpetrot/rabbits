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

#include "manager.h"

class Plugin;

/**
 * @brief Plugin factory base class.
 */
class PluginFactory {
private:
    PluginFactory(const PluginFactory&);
    PluginFactory & operator= (const PluginFactory&);

    static std::vector<PluginFactory*> *m_insts;

protected:
    PluginFactory() { 
        if (m_insts == NULL) {
            m_insts = new std::vector<PluginFactory*>;
        }
        m_insts->push_back(this); 
    }

public:
    static void register_plugins() {
        std::vector<PluginFactory*>::iterator it;

        if (m_insts == NULL) {
            return;
        }

        for (it = m_insts->begin(); it != m_insts->end(); it++) {
            PluginManager::get().register_plugin(*it);
        }
    }

    virtual ~PluginFactory() {}

    virtual Plugin *create() = 0;
    virtual std::string get_name() = 0;
};

#endif
