/*
 *  This file is part of Rabbits
 *  Copyright (C) 2017  Clement Deschamps and Luc Michel
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

#pragma once

#include <string>
#include <sstream>
#include <set>

#include <rabbits/platform/description.h>
#include <rabbits/platform/builder.h>

#include "client.h"
#include "../../backends/stub/stub.h"

class BackendInstance;

class PauseRequestListener {
public:
    virtual void pause_request() = 0;
};

class UniqueNameGenerator {
private:
    const char *m_prefix;
    int m_idx = 0;

public:
    UniqueNameGenerator(const char *prefix) : m_prefix(prefix) {}

    std::string operator() ()
    {
        std::stringstream ss;

        ss << m_prefix << m_idx++;
        return ss.str();
    }
};

class SignalElement {
public:
    enum Status {
        STA_NEW,
        STA_CREATED,
        STA_FAILURE,
        STA_DELETED
    };

    enum FailureReason {
        FAIL_NO_FAILURE,
        FAIL_INVALID_TYPE,
        FAIL_COMP_NOT_FOUND,
        FAIL_PORT_NOT_FOUND,
        FAIL_BINDING,
        FAIL_INTERNAL,
    };

protected:
    BackendInstance& m_parent;

    const std::string m_name;
    PlatformDescription m_descr;
    Status m_status = STA_NEW;
    FailureReason m_failure_reason = FAIL_NO_FAILURE;

public:
    SignalElement(BackendInstance &parent, const std::string &name, PlatformDescription &descr)
        : m_parent(parent), m_name(name), m_descr(descr) {}
    virtual ~SignalElement() {}

    void set_failure(FailureReason r)
    {
        m_status = STA_FAILURE;
        m_failure_reason = r;
    }

    void set_created() { m_status = STA_CREATED; }
    void set_deleted() { m_status = STA_DELETED; }

    const std::string & get_name() { return m_name; }
    PlatformDescription & get_description() { return m_descr; }

    SignalElement::Status get_status() const { return m_status; }
    SignalElement::FailureReason get_failure_reason() const { return m_failure_reason; }

    BackendInstance & get_parent() { return m_parent; }

    virtual void reconfigure(PlatformDescription &d) = 0;
};

class SignalGenerator : public SignalElement {
public:
    typedef std::shared_ptr<SignalGenerator> Ptr;

private:
    static UniqueNameGenerator m_unique_name;
    int m_cur_idx;

public:
    SignalGenerator(BackendInstance &parent, const std::string &name, PlatformDescription &d)
        : SignalElement(parent, name, d) {}

    static std::string get_unique_name() {
        return m_unique_name();
    }

    void reconfigure(PlatformDescription &d);
};

class SignalEvent : public SignalElement, public StubEventListener {
public:
    typedef std::shared_ptr<SignalEvent> Ptr;

    enum TriggerCondition {
        TRIG_ALWAYS,
        TRIG_EQ,
        TRIG_NE,
        TRIG_LT,
        TRIG_LE,
        TRIG_GT,
        TRIG_GE,
        TRIG_RANGE,
    };

    enum EventAction {
        EVENT_IDLE,
        EVENT_STOP,
        EVENT_PAUSE
    };

private:
    static UniqueNameGenerator m_unique_name;

protected:
    JsonConsoleClient::Ptr m_client;
    EventAction m_on_event = EVENT_IDLE;
    PauseRequestListener &m_pause_req_listener;

    virtual bool check_condition() = 0;

public:
    SignalEvent(BackendInstance &parent, const std::string &name, PlatformDescription &d,
                JsonConsoleClient::Ptr client, PauseRequestListener &l)
        : SignalElement(parent, name, d)
        , m_client(client)
        , m_pause_req_listener(l)
    {}

    static std::string get_unique_name() {
        return m_unique_name();
    }

    /* StubEventListener */
    void stub_event();

    virtual void reconfigure(PlatformDescription &d) = 0;
};

class BackendInstance {
private:
    static UniqueNameGenerator m_unique_name;
    static std::string get_unique_name() { return m_unique_name(); }

protected:
    const std::string m_name;

    const std::string m_comp_name;
    const std::string m_port_name;
    const std::string m_type;

    ConfigManager & m_config;

    bool m_elaboration_done = false;

    SignalGenerator::Ptr m_generator;
    std::set<SignalEvent::Ptr> m_events;

    StubBackendBase *m_backend = nullptr;
    PlatformBuilder *m_builder = nullptr;

    SignalElement::Status m_status = SignalElement::STA_NEW;
    SignalElement::FailureReason m_failure_reason = SignalElement::FAIL_NO_FAILURE;

    void apply_generator(SignalGenerator::Ptr g);
    void apply_event(SignalEvent::Ptr e);

    bool sanity_checks();
    bool create_backend();
    bool bind_backend();
    void apply_elements();

    void set_failure(SignalElement::FailureReason r);

public:
    BackendInstance(const std::string comp, const std::string port,
                    const std::string type, ConfigManager & config)
        : m_name(get_unique_name())
        , m_comp_name(comp)
        , m_port_name(port)
        , m_type(type)
        , m_config(config) {}

    virtual ~BackendInstance() {}

    SignalGenerator::Ptr create_generator(PlatformDescription &descr);
    SignalEvent::Ptr create_event(PlatformDescription &descr,
                                  JsonConsoleClient::Ptr, PauseRequestListener&);

    void delete_event(SignalEvent::Ptr);

    void elaborate(PlatformBuilder &builder);

    const std::string & get_name() const { return m_name; }

    void serialize_val(PlatformDescription &d)
    {
        std::string str_v;

        if (m_type == "bool") {
            bool v = m_backend->get_value<bool>();
            str_v = v ? "true" : "false";
        } else if (m_type == "double") {
            double v = m_backend->get_value<double>();
            str_v = v;
        } else {
            assert(false);
        }

        str_v = "{ \"value\": " + str_v + " }";
        d.load_json(str_v);
    }

    template <class T>
    T get_value() const
    {
        return m_backend->get_value<T>();
    }

    ConfigManager & get_config() { return m_config; }

    SignalElement::Status get_status() const { return m_status; }
    SignalElement::FailureReason get_failure_reason() const { return m_failure_reason; }

    void reconfigure(PlatformDescription &d);
};
