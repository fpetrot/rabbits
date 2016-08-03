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
class PlatformParser;

class PluginHook {
protected:
    PlatformDescription &m_descr;
    PlatformBuilder &m_builder;
    PlatformParser &m_parser;

public:
    PluginHook(PlatformDescription &descr, PlatformBuilder &builder, PlatformParser &parser)
        : m_descr(descr), m_builder(builder), m_parser(parser) {}

    /**
     * @brief Get the platform description.
     *
     * @return the platform description.
     */
    PlatformDescription& get_descr() const { return m_descr; }

    /**
     * @brief Get the platform builder.
     *
     * @return the platform builder.
     */
    PlatformBuilder& get_builder() const { return m_builder; }

    /**
     * @brief Get the platform parser.
     *
     * @return the platform parser.
     */
    PlatformParser& get_parser() const { return m_parser; }
};

/**
 * @brief Hook context used before the build starts.
 */
class PluginHookBeforeBuild : public PluginHook {
public:
    PluginHookBeforeBuild(PlatformDescription &descr, PlatformBuilder &builder,
                          PlatformParser &parser)
        : PluginHook(descr, builder, parser) {}
};

/**
 * @brief Hook context used after the components discovery step.
 */
class PluginHookAfterComponentDiscovery : public PluginHook {
public:
    PluginHookAfterComponentDiscovery (PlatformDescription &descr, PlatformBuilder &builder,
                                       PlatformParser &parser)
        : PluginHook(descr, builder, parser) {}
};

/**
 * @brief Hook context used after the components creation step.
 */
class PluginHookAfterComponentInst : public PluginHook {
public:
    PluginHookAfterComponentInst (PlatformDescription &descr,
                                  PlatformBuilder &builder, PlatformParser &parser)
        : PluginHook(descr, builder, parser) {}
};

/**
 * @brief Hook context used after the backends creation step.
 */
class PluginHookAfterBackendInst : public PluginHook {
public:
    PluginHookAfterBackendInst (PlatformDescription &descr,
                                PlatformBuilder &builder, PlatformParser &parser)
        : PluginHook(descr, builder, parser) {}
};

/**
 * @brief Hook context used after the ports binding step.
 */
class PluginHookAfterBindings : public PluginHook {
public:
    PluginHookAfterBindings (PlatformDescription &descr,
                                   PlatformBuilder &builder, PlatformParser &parser)
        : PluginHook(descr, builder, parser) {}
};

/**
 * @brief Hook context used at the end of the build.
 */
class PluginHookAfterBuild : public PluginHook {
public:
    PluginHookAfterBuild(PlatformDescription &descr, PlatformBuilder &builder,
                         PlatformParser &parser)
        : PluginHook(descr, builder, parser) {}
};

#endif
