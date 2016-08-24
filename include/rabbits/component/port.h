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

#ifndef _RABBITS_COMPONENT_PORT_H
#define _RABBITS_COMPONENT_PORT_H

#include <list>
#include <utility>
#include <string>
#include <map>
#include <vector>

#include "rabbits/config.h"

#ifdef RABBITS_WORKAROUND_CXX11_GCC_BUGS
# include <memory>
#else
# include <functional>
#endif

#include <systemc>

#include "connection_strategy.h"
#include "rabbits/rabbits_exception.h"
#include "rabbits/logger/has_logger.h"
#include "rabbits/logger.h"

class Port;


#ifdef RABBITS_WORKAROUND_CXX11_GCC_BUGS

/* std::function is broken in current GCC 4.9... */
class ScThreadCallbackFtor {
public:
    virtual ~ScThreadCallbackFtor() {}
    virtual void operator() () = 0;
};

typedef std::shared_ptr<ScThreadCallbackFtor> ScThreadCallback;

#else

typedef std::function< void() > ScThreadCallback;

#endif


class HasPortIface {
public:
    virtual void declare_port(Port &, const std::string &name) = 0;

    virtual bool port_exists(const std::string &name) const = 0;
    virtual Port& get_port(const std::string &name) = 0;

    typedef std::map<std::string, Port*>::iterator port_iterator;
    typedef std::map<std::string, Port*>::const_iterator const_port_iterator;

    virtual port_iterator port_begin() = 0;
    virtual port_iterator port_end() = 0;

    virtual const_port_iterator port_begin() const = 0;
    virtual const_port_iterator port_end() const = 0;

    virtual std::string hasport_name() const = 0;
    virtual Logger & hasport_getlogger(LogContext::value) const = 0;

    virtual void push_sc_thread(ScThreadCallback thread_callback) = 0;
};


class PortBindingListener {
public:
    virtual ~PortBindingListener() {}
    virtual void port_binding_event(Port &this_port, Port &peer_port) = 0;
};


class Port : public HasLoggerIface {
public:
    typedef std::pair<ConnectionStrategyBase*, ConnectionStrategyBase*> CSPair;
    typedef std::list< CSPair > CSPairs;

private:
    std::list<ConnectionStrategyBase*> m_cs;
    bool m_is_connected;
    std::vector<PortBindingListener*> m_listeners;

protected:
    std::string m_name;
    HasPortIface *m_parent = nullptr;

    void add_connection_strategy_front(ConnectionStrategyBase &cs)
    {
        m_cs.push_front(&cs);
    }

    void add_connection_strategy(ConnectionStrategyBase &cs)
    {
        m_cs.push_back(&cs);
    }

    void declare_parent(sc_core::sc_object *);
    void declare_parent(HasPortIface *);

    void add_attr_to_parent(const std::string & key, const std::string & value);

    template <class Callback>
    void push_thread_to_parent(Callback c) {
        if (!m_parent) {
            return;
        }

        m_parent->push_sc_thread(ScThreadCallback(c));
    }

    virtual void connect(Port& p, CSPairs pairs, PlatformDescription &d)
    {

        ConnectionStrategyBase::BindingResult r = ConnectionStrategyBase::BINDING_TRY_NEXT;

        while (r == ConnectionStrategyBase::BINDING_TRY_NEXT && !pairs.empty()) {
            CSPair pair = pairs.front();
            pairs.pop_front();

            r = pair.first->bind(*pair.second, ConnectionStrategyBase::PEER, d);

            switch (r) {
            case ConnectionStrategyBase::BINDING_OK:
                selected_strategy(*pair.first);
                p.selected_strategy(*pair.second);
                m_is_connected = p.m_is_connected = true;
                return;

            case ConnectionStrategyBase::BINDING_HIERARCHICAL_TYPE_MISMATCH:
                /* Should not happen for peer binding */
                abort();

            case ConnectionStrategyBase::BINDING_ERROR:
                MLOG(APP, WRN) << "Error while binding " << full_name()
                              << " to " << p.full_name() << "\n";
                break;

            case ConnectionStrategyBase::BINDING_TRY_NEXT:
                break;
            }

        }

    }

    void dispatch_binding_ev(Port &peer)
    {
        for (auto l: m_listeners) {
            l->port_binding_event(*this, peer);
        }
    }

public:
    explicit Port(const std::string &name) : m_is_connected(false), m_name(name) {}

    virtual ~Port() {}

    const std::list<ConnectionStrategyBase*> & get_connection_strategies() const {
        return m_cs;
    }

    bool is_connected() const { return m_is_connected; }

    ConnectionStrategyBase * is_compatible_with(const ConnectionStrategyBase &cs)
    {
        const std::list<ConnectionStrategyBase*> & s = get_connection_strategies();

        for (ConnectionStrategyBase *ocs : s) {
            if (ocs->is_compatible_with(cs)) {
                return ocs;
            }
        }

        return NULL;
    }

    bool is_connectable_to(Port& p, CSPairs &pairs)
    {
        const std::list<ConnectionStrategyBase*> & s = get_connection_strategies();

        for (ConnectionStrategyBase *cs : s) {
            if (ConnectionStrategyBase *ocs = p.is_compatible_with(*cs)) {
                pairs.push_back(std::make_pair(cs, ocs));
            }
        }

        return pairs.size() != 0;
    }

    virtual bool connect(Port &p,
                         PlatformDescription &d = PlatformDescription::INVALID_DESCRIPTION)
    {
        CSPairs pairs;

        if (!is_connectable_to(p, pairs)) {
            return false;
        }

        connect(p, pairs, d);
        dispatch_binding_ev(p);
        p.dispatch_binding_ev(*this);
        return true;
    }

    void bind(Port& parent)
    {
        CSPairs pairs;

        if (!is_connectable_to(parent, pairs)) {
            MLOG(APP, WRN) << full_name() << " is not connectable to " << parent.full_name() << "\n";
            return;
        }

        ConnectionStrategyBase::BindingResult r = ConnectionStrategyBase::BINDING_TRY_NEXT;

        while (r == ConnectionStrategyBase::BINDING_TRY_NEXT && !pairs.empty()) {
            CSPair pair = pairs.front();
            pairs.pop_front();

            r = pair.first->bind(*pair.second, ConnectionStrategyBase::HIERARCHICAL);

            switch (r) {
            case ConnectionStrategyBase::BINDING_OK:
                m_is_connected = true;
                dispatch_binding_ev(parent);
                parent.dispatch_binding_ev(*this);
                return;

            case ConnectionStrategyBase::BINDING_HIERARCHICAL_TYPE_MISMATCH:
                MLOG(APP, WRN) << full_name() << " is not hierarchically connectable to "
                              << parent.full_name() << "\n";
                return;

            case ConnectionStrategyBase::BINDING_ERROR:
                MLOG(APP, WRN) << "Error while hierarchical binding of " << full_name()
                              << " with " << parent.full_name() << "\n";
                break;

            case ConnectionStrategyBase::BINDING_TRY_NEXT:
                break;
            }
        }

    }

    void operator() (Port& p) { bind(p); }

    virtual void selected_strategy(ConnectionStrategyBase &cs) {}

    const std::string & name() { return m_name; }
    std::string full_name()
    {
        if (!m_parent) {
            return name();
        }

        return m_parent->hasport_name() + "." + name();
    }

    HasPortIface* get_parent() { return m_parent; }

    /* HasLoggerIface */
    Logger & get_logger(LogContext::value context) const {
        if (m_parent) {
            return m_parent->hasport_getlogger(context);
        } else {
            return get_logger(context);
        }
    }

    void register_binding_listener(PortBindingListener &l)
    {
        m_listeners.push_back(&l);
    }

    virtual void before_end_of_elaboration() {}
    virtual void end_of_elaboration() {}
    virtual void start_of_simulation() {}
    virtual void end_of_simulation() {}
};

class PortNotFoundException : public RabbitsException {
protected:
    std::string make_what(std::string name) { return "Port `" + name + "' not found."; }
public:
    PortNotFoundException(const std::string &name) : RabbitsException(make_what(name)) {}
    virtual ~PortNotFoundException() throw() {}
};

#endif
