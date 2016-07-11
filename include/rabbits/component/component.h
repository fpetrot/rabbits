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
#include <memory>
#include <fstream>

#include "component_base.h"
#include "rabbits/rabbits_exception.h"

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
    enum LogTarget {
        LT_STDOUT, LT_STDERR, LT_FILE
    };

    typedef std::unique_ptr<std::fstream> LogFile;
    typedef std::map<std::string, LogFile> LogFiles;

    SC_HAS_PROCESS(Component);

protected:
    std::map<std::string, Port*> m_ports;
    std::vector< std::function<void() > > m_pushed_threads;
    LogFiles m_log_files;

    Attributes m_attributes;

    mutable Logger m_loggers[LogContext::LASTLOGCONTEXT];

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

    LogTarget get_log_target(const std::string target_s)
    {
        if (target_s == "stdout") {
            return LT_STDOUT;
        } else if (target_s == "stderr") {
            return LT_STDERR;
        } else if (target_s == "file") {
            return LT_FILE;
        }

        LOG(APP, ERR) << "Ignoring invalid log target " << target_s << "\n";
        return LT_STDERR;
    }

    LogLevel::value get_log_level(const std::string level_s)
    {
        if (level_s == "debug") {
            return LogLevel::DEBUG;
        } else if (level_s == "info") {
            return LogLevel::INFO;
        } else if (level_s == "warning") {
            return LogLevel::WARNING;
        } else if (level_s == "error") {
            return LogLevel::ERROR;
        }

        LOG(APP, ERR) << "Ignoring invalid log level " << level_s << "\n";
        return LogLevel::INFO;
    }

    std::fstream* open_file(const std::string &fn)
    {
        std::fstream* ret = nullptr;

        if (m_log_files.find(fn) != m_log_files.end()) {
            if (!m_log_files[fn]) {
                return nullptr;
            }
            return m_log_files[fn].get();
        }

        ret = new std::fstream(fn, std::fstream::out | std::fstream::trunc);

        m_log_files[fn].reset(ret);

        return ret;
    }

    void setup_logger_banner(Logger &l)
    {
        std::stringstream banner;
        banner << "[" << name() << "]";
        l.set_custom_banner(banner.str());

        l.set_custom_banner([] (Logger &l, const std::string &banner)
        {
            l << format::cyan << banner << format::reset;
        });
    }

    void setup_logger(Logger &l, LogTarget target, LogLevel::value lvl,
                      const std::string log_file)
    {
        setup_logger_banner(l);

        switch (target) {
        case LT_STDOUT:
            l.set_streams(&std::cout);
            break;

        case LT_FILE:
            {
                std::fstream* file = open_file(log_file);

                if (!*file) {
                    LOG(APP, ERR) << "Unable to open log file "
                        << log_file << ". Falling back to stderr\n";
                } else {
                    l.set_streams(file);
                }
            }
            break;

        case LT_STDERR:
            /* Default */
            break;
        }

        l.set_log_level(lvl);
    }

    bool logger_is_custom()
    {
        return (!m_params["log-target"].is_default())
            || (!m_params["log-level"].is_default())
            || (!m_params["log-file"].is_default())
            || (!m_params["debug"].is_default());
    }

    void setup_loggers()
    {
        LogTarget log_target;
        LogLevel::value log_level;
        std::string log_file;
        bool debug = false;
        bool no_custom = false;

        if (m_params.exists("log-file")) {
            no_custom = !logger_is_custom();
            log_target = get_log_target(m_params["log-target"].as<std::string>());
            log_level = get_log_level(m_params["log-level"].as<std::string>());
            log_file = m_params["log-file"].as<std::string>();
            debug = m_params["debug"].as<bool>();
        } else {
            log_target = LT_STDERR;
            log_level = LogLevel::INFO;
            log_file = "";
        }

        if (debug) {
            log_level = LogLevel::DEBUG;
        }

        for (int i = 0; i < LogContext::LASTLOGCONTEXT; i++) {
            Logger::get_root_logger(LogContext::value(i)).set_child(m_loggers[i]);

            if (no_custom) {
                setup_logger_banner(m_loggers[i]);
            } else {
                setup_logger(m_loggers[i], log_target, log_level, log_file);
            }
        }
    }

    void init()
    {
        if (m_params.exists("log-file")) {
            m_params["log-file"].set_default(std::string(name()) + ".log");
        }

        setup_loggers();
    }

public:
    Component(sc_core::sc_module_name name, const ComponentParameters &params)
        : ComponentBase(name, params) { init(); }

    Component(sc_core::sc_module_name name) : ComponentBase(name) { init(); }

    virtual ~Component() {}

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

    /* HasLoggerIface */
    Logger & get_logger(LogContext::value context) const { return m_loggers[context]; }
};

#endif
