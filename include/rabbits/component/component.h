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
#include <memory>
#include <fstream>

#include <systemc>

#include "rabbits/module/module.h"

#include "port.h"
#include "rabbits/rabbits_exception.h"
#include "rabbits/datatypes/address_range.h"
#include "rabbits/platform/description.h"
#include "rabbits/config/manager.h"
#include "rabbits/logger/wrapper.h"


/**
 * @brief Exception raised when a named component has not been found.
 */
class ComponentNotFoundException : public RabbitsException {
protected:
    std::string make_what(std::string comp) { return "Component `" + comp + "` not found."; }
public:
    explicit ComponentNotFoundException(const std::string & comp) : RabbitsException(make_what(comp)) {}
    virtual ~ComponentNotFoundException() throw() {}
};


class HasAttributesIface
{
public:
    virtual void add_attr(const std::string & key, const std::string & value) = 0;
    virtual bool has_attr(const std::string & key) = 0;
    virtual std::vector<std::string> get_attr(const std::string & key) = 0;
};


/**
 * @brief Component base class.
 */
class ComponentBase
    : public sc_core::sc_module
    , public ModuleIface
    , public HasPortIface
    , public HasAttributesIface {
public:
    ComponentBase(sc_core::sc_module_name name) : sc_core::sc_module(name) {}
};


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
    typedef std::map< std::string, std::vector<std::string> > Attributes;

    SC_HAS_PROCESS(Component);

protected:
    std::string m_name;
    Parameters m_params;
    ConfigManager &m_config;

    LoggerWrapper m_loggers;

    std::map<std::string, Port*> m_ports;

    std::vector<ScThreadCallback> m_pushed_threads;

    Attributes m_attributes;

    void pushed_threads_entry()
    {
        auto t = m_pushed_threads.back();
        m_pushed_threads.pop_back();

#ifdef RABBITS_WORKAROUND_CXX11_GCC_BUGS
        (*t)();
#else
        t();
#endif
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

    void init()
    {
        m_name = std::string(basename());
        m_params.set_module(*this);
    }

public:
    Component(sc_core::sc_module_name n, const Parameters &params, ConfigManager &config)
        : ComponentBase(n)
        , m_params(params)
        , m_config(config)
        , m_loggers(std::string(name()), config, m_params, config)
    {
        init();
    }

    Component(sc_core::sc_module_name n, ConfigManager &config)
        : ComponentBase(n)
        , m_config(config)
        , m_loggers(std::string(name()), config, m_params, config)
    {
        init();
    }

    virtual ~Component() {}

    /* ModuleIface */
    const std::string & get_name() const { return m_name; }
    const Namespace & get_namespace() const { return *(m_params.get_namespace()); }
    const std::string get_full_name() const { return m_params.get_namespace()->get_name() + "." + get_name(); }

    /* HasPortIface */
    virtual void declare_port(Port &p, const std::string &name) { m_ports[name] = &p; }
    virtual bool port_exists(const std::string &name) const { return (m_ports.find(name) != m_ports.end()); }

    virtual Port& get_port(const std::string &name)
    {
        if (!port_exists(name)) {
            throw PortNotFoundException(name);
        }
        return *m_ports[name];
    }

    virtual port_iterator port_begin() { return m_ports.begin(); }
    virtual port_iterator port_end() { return m_ports.end(); }
    virtual const_port_iterator port_begin() const { return m_ports.begin(); }
    virtual const_port_iterator port_end() const { return m_ports.end(); }
    virtual std::string hasport_name() const { return name(); }
    virtual Logger & hasport_getlogger(LogContext::value context) const {
        return get_logger(context);
    }
    virtual void push_sc_thread(ScThreadCallback cb) {
        m_pushed_threads.push_back(cb);
    }


    /* HasParametersIface */
    const Parameters & get_params() const { return m_params; }

    /* HasAttributesIface */
    void add_attr(const std::string & key, const std::string & value)
    {
        m_attributes[key].push_back(value);
    }

    bool has_attr(const std::string & key)
    {
        return (m_attributes.find(key) != m_attributes.end());
    }

    std::vector<std::string> get_attr(const std::string & key)
    {
        if (!has_attr(key)) {
            return std::vector<std::string>();
        }

        return m_attributes[key];
    }

    /* HasLoggerIface */
    Logger & get_logger(LogContext::value context) const { return m_loggers.get_logger(context); }

    /* HasConfigIface */
    ConfigManager & get_config() const { return m_config; }
};

#endif
