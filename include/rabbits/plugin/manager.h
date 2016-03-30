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
 * @brief PluginManager class declaration.
 */

#ifndef _UTILS_PLUGIN_MANAGER_H
#define _UTILS_PLUGIN_MANAGER_H

#include <vector>

#include "plugin.h"

class PluginFactory;

/**
 * @brief The plugin manager.
 */
class PluginManager {
private:
    static PluginManager *m_inst;

protected:
    std::vector<Plugin*> m_plugins;

    PluginManager() {}

public:
    virtual ~PluginManager() {}

    /**
     * @brief Return the singleton instance of the plugin manager.
     *
     * @return the singleton instance of the plugin manager.
     */
    static PluginManager& get();

    /**
     * @brief Register a new plugin into the manager.
     *
     * @param[in] PluginFactory The factory of the plugin to register.
     */
    void register_plugin(PluginFactory*);

    /**
     * @brief Run the hook corresponding to the hook type HOOK.
     *
     * @tparam HOOK the hook type.
     * @param[in] h the hook context instance.
     */
    template <typename HOOK>
    void run_hook(const HOOK &h);
};


template <typename HOOK>
inline void PluginManager::run_hook(const HOOK &hook)
{
    std::vector<Plugin*>::iterator it;

    for (it = m_plugins.begin(); it != m_plugins.end(); it++) {
        Plugin *p = *it;
        p->hook(hook);
    }
}

#endif
