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
 * @file component.h
 * Component class declaration
 */

#ifndef _UTILS_COMPONENT_COMPONENT_H
#define _UTILS_COMPONENT_COMPONENT_H

#include <map>
#include <utility>
#include <vector>
#include <sstream>
#include <systemc>

#include "component_base.h"
#include "rabbits/rabbits_exception.h"
#include "rabbits/datatypes/indexed_pool.h"

/**
 * @brief A rabbits component.
 *
 * This class represents a rabbits component. It can have in and out IRQs, and
 * children components.
 *
 * Not connected IRQs at end of elaboration are automatically stubbed by
 * connecting a signal to them.
 *
 * @see Slave, Master
 * @see IrqIn, IrqOut
 */
class Component : public ComponentBase {
public:
    SC_HAS_PROCESS(Component);

protected:
    IndexedPool<std::string, Port*> m_ports;

    std::vector< std::function<void() > > m_pushed_threads;

    Attributes m_attributes;

    void pushed_threads_entry()
    {
        auto &t = m_pushed_threads.back();
        m_pushed_threads.pop_back();

        t();
    }

    /* Macro to avoid SystemC warnings about name duplication */
#define indexed_SC_THREAD(func)                                \
    declare_thread_process(func ## _handle,                    \
                           sc_core::sc_gen_unique_name(#func), \
                           SC_CURRENT_USER_MODULE,             \
                           func)

    void create_pushed_threads()
    {
        for (unsigned int i = 0; i < m_pushed_threads.size(); i++) {
            indexed_SC_THREAD(pushed_threads_entry);
        }
    }

#undef indexed_SC_THREAD

    virtual void before_end_of_elaboration()
    {
        ComponentBase::before_end_of_elaboration();

        create_pushed_threads(); 

        for (const auto & p : m_ports) {
            p.second->before_end_of_elaboration();
        }
    }

    virtual void end_of_elaboration()
    {
        ComponentBase::end_of_elaboration();

        for (const auto & p : m_ports) {
            p.second->end_of_elaboration();
        }
    }

    virtual void start_of_simulation()
    {
        ComponentBase::start_of_simulation();

        for (const auto & p : m_ports) {
            p.second->start_of_simulation();
        }
    }

    virtual void end_of_simulation()
    {
        ComponentBase::end_of_simulation();

        for (const auto & p : m_ports) {
            p.second->end_of_simulation();
        }
    }

public:
    Component(sc_core::sc_module_name name, const ComponentParameters &params)
        : ComponentBase(name, params) {}

    Component(sc_core::sc_module_name name) : ComponentBase(name) {}

    virtual ~Component() {}

    /* HasPortIface */
    virtual void declare_port(Port &p, const std::string &name) { m_ports.add(name, &p); }
    virtual bool port_exists(const std::string &name) const { return m_ports.exists(name); }
    virtual Port& get_port(const std::string &name) { return *m_ports[name]; }
    virtual bool port_exists(int index) const { return m_ports.exists(index); }
    virtual Port& get_port(int index) { return *m_ports[index]; }
    virtual port_iterator port_begin() { return m_ports.begin(); }
    virtual port_iterator port_end() { return m_ports.end(); }
    virtual const_port_iterator port_begin() const { return m_ports.begin(); }
    virtual const_port_iterator port_end() const { return m_ports.end(); }
    virtual std::string hasport_name() const { return name(); }
    virtual void push_sc_thread(std::function<void()> cb) {
        m_pushed_threads.push_back(cb);
    }
    
    /* HasAttributesIface */
    void add_attr(const std::string & key, const std::string & value)
    {
        m_attributes[key] = value;
    }

    bool has_attr(const std::string & key)
    {
        return (m_attributes.find(key) != m_attributes.end());
    }

    std::string get_attr(const std::string & key)
    {
        if (!has_attr(key)) {
            return "";
        }

        return m_attributes[key];
    }
};

#endif
