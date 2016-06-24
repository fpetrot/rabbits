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

#include "rabbits/component/manager.h"
#include "rabbits/component/component.h"
#include "rabbits/component/debug_initiator.h"

class PlatformDescription;
class AddressRange;

/**
 * @brief The platform builder.
 *
 * Create a platform from a PlatformDescription.
 * The instance of this class will be the top-level SystemC module, instancing
 * and connecting the components altogether during elaboration.
 */
class PlatformBuilder : public sc_core::sc_module {
public:
    typedef std::map<std::string, ComponentBase*>::iterator comp_iterator;
    typedef std::map<std::string, ComponentBase*>::const_iterator const_comp_iterator;
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

    std::map<std::string, ComponentBase*> m_components;
    DebugInitiator *m_dbg;

    void create_components(PlatformDescription &descr, CreationStage::value);

    void do_bindings(PlatformDescription &descr);
    void do_bindings(ComponentBase &c, PlatformDescription &descr);
    void do_binding(Port &p, PlatformDescription &descr);
    void do_binding(Port &p0, Port &p1, PlatformDescription &descr);

    void create_dbg_init();

public:
    SC_HAS_PROCESS(PlatformBuilder);

    /**
     * @brief Construct a platform builder that will build a platform according to the description descr.
     *
     * @param name Name of the SystemC module.
     * @param descr Platform description.
     */
    PlatformBuilder(sc_core::sc_module_name name, PlatformDescription &descr);
    virtual ~PlatformBuilder();

    /**
     * @brief Return the DebugInitiator instance connected to the platform bus.
     *
     * @return the DebugInitiator instance connected to the platform bus.
     */
    DebugInitiator& get_dbg_init() { return *m_dbg; }

    /**
     * @brief Return an iterator to the first component of the platform.
     *
     * @return an iterator to the first component of the platform.
     */
    comp_iterator comp_begin() { return m_components.begin(); }

    /**
     * @brief Return an iterator to the <i>past-the-end</i> component of the platform.
     *
     * @return an iterator to the <i>past-the-end</i> component of the platform.
     */
    comp_iterator comp_end() { return m_components.end(); }

    /**
     * @brief Return a constant iterator to the first component of the platform.
     *
     * @return a constant iterator to the first component of the platform.
     */
    const_comp_iterator comp_begin() const { return m_components.begin(); }

    /**
     * @brief Return a constant iterator to the <i>past-the-end</i> component of the platform.
     *
     * @return a constant iterator to the <i>past-the-end</i> component of the platform.
     */
    const_comp_iterator comp_end() const { return m_components.end(); }

    /**
     * @brief Return true if the component of the given name exists.
     *
     * @param name The component name.
     *
     * @return true if the component exists, false otherwise.
     */
    bool comp_exists(const std::string &name) const { return m_components.find(name) != m_components.end(); }

    void find_comp_by_attr(const std::string &key, std::vector<ComponentBase*> &out);

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
};

#endif
