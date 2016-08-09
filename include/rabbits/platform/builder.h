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
 * @file builder.h
 * @brief PlatformBuilder class declaration.
 */

#ifndef _UTILS_PLATFORM_BUILDER_H
#define _UTILS_PLATFORM_BUILDER_H

#include <systemc>

#include <vector>
#include <map>

#include "rabbits/config/manager.h"
#include "rabbits/component/manager.h"
#include "rabbits/component/component.h"
#include "rabbits/component/debug_initiator.h"
#include "rabbits/platform/parser.h"

class PlatformDescription;
class AddressRange;

/**
 * @brief The platform builder.
 *
 * Create a platform from a PlatformDescription.
 * The instance of this class will be the top-level SystemC module, instancing
 * and connecting the components altogether during elaboration.
 */
class PlatformBuilder : public sc_core::sc_module, public HasConfigIface {
public:
    typedef std::map<std::string, ComponentBase*>::iterator comp_iterator;
    typedef std::map<std::string, ComponentBase*>::const_iterator const_comp_iterator;

    typedef std::map<std::string, PluginBase*> Plugins;
    typedef std::map<std::string, ComponentBase*> Components;
    typedef std::map<std::string, ModuleIface*> Modules;

protected:
    /**
     * @brief The components creation stages.
     */
    struct CreationStage {
        enum value {
            DISCOVER, /**< First stage, no creation is performed yet. */
            CREATE    /**< Second stage, actual creation happens here. */
        };
    };

    ConfigManager &m_config;

    PlatformParser m_parser;

    /* Instanciated modules */
    Plugins m_plugins;
    Components m_components;
    Components m_backends;

    /* Instanciated modules, refered as abstract ModuleIface */
    Modules m_modules[Namespace::COUNT];

    DebugInitiator *m_dbg = nullptr;

    void create_plugins(PlatformParser &p);
    template <class HOOK> void run_hooks(HOOK &&hook);

    void create_components(PlatformParser &p, CreationStage::value);
    void create_backends(PlatformParser &p);

    void do_bindings(PlatformParser &p);
    void do_bindings(std::shared_ptr<ParserNodeModuleWithPorts> node);
    void do_binding(Port &p0, Port &p1, PlatformDescription &descr);

    void create_dbg_init(PlatformParser&);

public:
    SC_HAS_PROCESS(PlatformBuilder);

    /**
     * @brief Construct a platform builder that will build a platform according to the description descr.
     *
     * @param name Name of the SystemC module.
     * @param descr Platform description.
     */
    PlatformBuilder(sc_core::sc_module_name name, PlatformDescription &platform,
                    ConfigManager &config);

    PlatformBuilder(sc_core::sc_module_name name, ConfigManager &config);

    virtual ~PlatformBuilder();

    /**
     * @brief Return the DebugInitiator instance connected to the platform bus.
     *
     * @return the DebugInitiator instance connected to the platform bus.
     */
    DebugInitiator& get_dbg_init() { return *m_dbg; }

    /**
     * @brief Return true if the component of the given namespace and name exists.
     *
     * @param[in] namespace The component namespace.
     * @param[in] name The component name.
     *
     * @return true if the component exists, false otherwise.
     */
    bool comp_exists(const Namespace &ns, const std::string &name) const
    {
        switch(ns.get_id()) {
        case Namespace::COMPONENT:
            return m_components.find(name) != m_components.end();
        case Namespace::BACKEND:
        return m_backends.find(name) != m_backends.end();
        default:
            return false;
        }

    }

    /**
     * @brief Return true if the component of the given name exists in the
     * COMPONENT namespace.
     *
     * @param name The component name.
     *
     * @return true if the component exists, false otherwise.
     */
    bool comp_exists(const std::string &name) const
    {
        return comp_exists(Namespace::get(Namespace::COMPONENT), name);
    }

    /**
     * @brief Return the component of the given namespace and name.
     *
     * @param[in] namespace The component namespace.
     * @param[in] name The component name.
     *
     * @return component instance.
     *
     * @throw ComponentNotFoundException if the component does not exists.
     */
    ComponentBase & get_comp(const Namespace &ns, const std::string &name) {
        if (!comp_exists(ns, name)) {
            throw ComponentNotFoundException(name);
        }

        switch(ns.get_id()) {
        case Namespace::COMPONENT:
            return *m_components[name];
        case Namespace::BACKEND:
            return *m_backends[name];
        default:
            throw ComponentNotFoundException(name);
        }
    }

    /**
     * @brief Return the component of the given name.
     *
     * @param name The component name.
     *
     * @return component instance.
     *
     * @throw ComponentNotFoundException if the component does not exists.
     */
    ComponentBase & get_comp(const std::string &name) {
        if (!comp_exists(name)) {
            throw ComponentNotFoundException(name);
        }

        return *m_components[name];
    }

    /**
     * @brief Return true if the built platform contains no components.
     *
     * @return true if the built platform contains no components.
     */
    bool is_empty() const { return m_components.empty(); }

    const Components get_components() const { return m_components; }
    const Components get_backends() const { return m_backends; }
    const Plugins get_plugins() const { return m_plugins; }

    void add_component(ComponentBase *c);
    void add_backend(ComponentBase *c);
    void add_plugin(PluginBase *p);

    /* HasConfigIface */
    ConfigManager & get_config() const { return m_config; }
};

#endif
