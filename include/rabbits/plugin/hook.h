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
 * @brief Plugin hooks classes declaration.
 */

#ifndef _UTILS_PLUGIN_HOOK_H
#define _UTILS_PLUGIN_HOOK_H

class PlatformDescription;
class PlatformBuilder;

/**
 * @brief Hook context used before the build starts.
 */
class PluginHookBeforeBuild {
protected:
    PlatformDescription *m_descr;
    PlatformBuilder *m_builder;

public:
    PluginHookBeforeBuild(PlatformDescription *descr, PlatformBuilder *builder) 
        : m_descr(descr), m_builder(builder) {}

    /**
     * @brief Get the platform description.
     *
     * @return the platform description.
     */
    PlatformDescription* get_descr() const { return m_descr; }

    /**
     * @brief Get the platform builder.
     *
     * @return the platform builder.
     */
    PlatformBuilder* get_builder() const { return m_builder; }
};

/**
 * @brief Hook context used after the components discovery step.
 */
class PluginHookAfterComponentDiscovery {
protected:
    PlatformDescription *m_descr;
    PlatformBuilder *m_builder;

public:
    PluginHookAfterComponentDiscovery (PlatformDescription *descr, PlatformBuilder *builder) 
        : m_descr(descr), m_builder(builder) {}

    /**
     * @brief Get the platform description.
     *
     * @return the platform description.
     */
    PlatformDescription* get_descr() const { return m_descr; }

    /**
     * @brief Get the platform builder.
     *
     * @return the platform builder.
     */
    PlatformBuilder* get_builder() const { return m_builder; }
};

/**
 * @brief Hook context used after the components creation step.
 */
class PluginHookAfterComponentInst {
protected:
    PlatformDescription *m_descr;
    PlatformBuilder *m_builder;

public:
    PluginHookAfterComponentInst (PlatformDescription *descr, PlatformBuilder *builder) 
        : m_descr(descr), m_builder(builder) {}

    /**
     * @brief Get the platform description.
     *
     * @return the platform description.
     */
    PlatformDescription* get_descr() const { return m_descr; }

    /**
     * @brief Get the platform builder.
     *
     * @return the platform builder.
     */
    PlatformBuilder* get_builder() const { return m_builder; }
};

/**
 * @brief Hook context used after the components connection step.
 */
class PluginHookAfterBusConnections {
protected:
    PlatformDescription *m_descr;
    PlatformBuilder *m_builder;

public:
    PluginHookAfterBusConnections (PlatformDescription *descr, PlatformBuilder *builder) 
        : m_descr(descr), m_builder(builder) {}

    /**
     * @brief Get the platform description.
     *
     * @return the platform description.
     */
    PlatformDescription* get_descr() const { return m_descr; }

    /**
     * @brief Get the platform builder.
     *
     * @return the platform builder.
     */
    PlatformBuilder* get_builder() const { return m_builder; }
};

/**
 * @brief Hook context used at the end of the build.
 */
class PluginHookAfterBuild {
protected:
    PlatformDescription *m_descr;
    PlatformBuilder *m_builder;

public:
    PluginHookAfterBuild(PlatformDescription *descr, PlatformBuilder *builder) 
        : m_descr(descr), m_builder(builder) {}

    /**
     * @brief Get the platform description.
     *
     * @return the platform description.
     */
    PlatformDescription* get_descr() const { return m_descr; }

    /**
     * @brief Get the platform builder.
     *
     * @return the platform builder.
     */
    PlatformBuilder* get_builder() const { return m_builder; }
};

#endif
