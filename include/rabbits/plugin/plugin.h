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
 * @file plugin.h
 * @brief Plugin class declaration.
 */

#ifndef _UTILS_PLUGIN_PLUGIN_H
#define _UTILS_PLUGIN_PLUGIN_H

#include "hook.h"

/**
 * @brief The plugin base class.
 *
 * A plugin must inherit this class and override hooks methods to be called at
 * desired platform creation stages.
 */
class Plugin {
public:
    virtual ~Plugin() {}

    /**
     * @brief Hook called before the build starts.
     *
     * @param[in] h Hook context.
     */
    virtual void hook(const PluginHookBeforeBuild& h) {}

    /**
     * @brief Hook called after the components discovery step.
     *
     * @param[in] h Hook context.
     */
    virtual void hook(const PluginHookAfterComponentDiscovery& h) {}

    /**
     * @brief Hook called after the components creation step.
     *
     * @param[in] h Hook context.
     */
    virtual void hook(const PluginHookAfterComponentInst& h) {}

    /**
     * @brief Hook called after the components connection step.
     *
     * @param[in] h Hook context.
     */
    virtual void hook(const PluginHookAfterBusConnections& h) {}

    /**
     * @brief Hook called at the end of the build.
     *
     * @param[in] h Hook context.
     */
    virtual void hook(const PluginHookAfterBuild& h) {}
};

#endif
