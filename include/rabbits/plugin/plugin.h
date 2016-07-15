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

#include "rabbits/config/manager.h"
#include "rabbits/module/module.h"
#include "rabbits/logger/wrapper.h"
#include "hook.h"

/**
 * @brief The plugin base class.
 *
 * A plugin must inherit this class and override hooks methods to be called at
 * desired platform creation stages.
 */
class PluginBase : public ModuleIface {
public:
    /**
     * @brief Hook called before the build starts.
     *
     * @param[in] h Hook context.
     */
    virtual void hook(const PluginHookBeforeBuild& h) = 0;

    /**
     * @brief Hook called after the components discovery step.
     *
     * @param[in] h Hook context.
     */
    virtual void hook(const PluginHookAfterComponentDiscovery& h) = 0;

    /**
     * @brief Hook called after the components creation step.
     *
     * @param[in] h Hook context.
     */
    virtual void hook(const PluginHookAfterComponentInst& h) = 0;

    /**
     * @brief Hook called after the components connection step.
     *
     * @param[in] h Hook context.
     */
    virtual void hook(const PluginHookAfterBusConnections& h) = 0;

    /**
     * @brief Hook called at the end of the build.
     *
     * @param[in] h Hook context.
     */
    virtual void hook(const PluginHookAfterBuild& h) = 0;
};

class Plugin : public PluginBase {
protected:
    std::string m_name;
    Parameters m_params;
    ConfigManager &m_config;

    LoggerWrapper m_loggers;

public:
    Plugin(const std::string & name, const Parameters &params, ConfigManager &config)
        : m_name(name), m_params(params), m_config(config)
        , m_loggers(name, config, m_params, config)
    {
        m_params.set_module(*this);
    }

    virtual ~Plugin() {}

    const std::string & get_name() const { return m_name; }

    virtual void hook(const PluginHookBeforeBuild& h) {}
    virtual void hook(const PluginHookAfterComponentDiscovery& h) {}
    virtual void hook(const PluginHookAfterComponentInst& h) {}
    virtual void hook(const PluginHookAfterBusConnections& h) {}
    virtual void hook(const PluginHookAfterBuild& h) {}

    /* HasParametersIface */
    const Parameters & get_params() const { return m_params; }

    /* HasLoggerIface */
    Logger & get_logger(LogContext::value context) const { return m_loggers.get_logger(context); }

    /* HasConfigIface */
    ConfigManager & get_config() const { return m_config; }
};

#endif
