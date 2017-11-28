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

#include <sstream>

#include "backend.h"

using std::string;
using std::stringstream;
using std::vector;

UniqueNameGenerator SignalGenerator::m_unique_name("generator");
UniqueNameGenerator SignalEvent::m_unique_name("trigger");
UniqueNameGenerator BackendInstance::m_unique_name("backend");

void SignalGenerator::reconfigure(PlatformDescription &d)
{
    m_descr = d;
    m_parent.reconfigure(d);
}

void SignalEvent::stub_event()
{
    if (check_condition()) {
        m_client->signal_event(*this);

        switch (m_on_event) {
        case EVENT_IDLE:
            break;

        case EVENT_STOP:
            sc_core::sc_stop();
            break;

        case EVENT_PAUSE:
            m_pause_req_listener.handle_next_pause_event();
            sc_core::sc_pause();
            break;
        }
    }
}

template <class T>
class TypedSignalEvent : public SignalEvent {
protected:
    TriggerCondition m_cond;
    T m_value;
    std::vector<T> m_range;

    bool check_condition();

    void configure_event_action(PlatformDescription &d);
    void configure_cond(PlatformDescription &d);

public:
    TypedSignalEvent(BackendInstance &parent,
                     const std::string &name, PlatformDescription &d,
                     JsonConsoleClient::Ptr client, PauseRequestListener &l)
        : SignalEvent(parent, name, d, client, l)
    {
        reconfigure(d);
    }

    void reconfigure(PlatformDescription &d);
};


template <class T>
void TypedSignalEvent<T>::configure_event_action(PlatformDescription &d)
{
    if (d.is_invalid()) {
        m_on_event = EVENT_IDLE;
        return;
    }

    if (!d.is_scalar()) {
        throw ConfigFailureException("invalid on-event value");
    }

    const string s = d.as<string>();

    if (s == "continue") {
        m_on_event = EVENT_IDLE;
    } else if (s == "pause") {
        m_on_event = EVENT_PAUSE;
    } else if (s == "stop") {
        m_on_event = EVENT_STOP;
    } else {
        throw ConfigFailureException(string("invalid on-event value `") + s + "'");
    }
}

template <class T>
void TypedSignalEvent<T>::configure_cond(PlatformDescription &d)
{
    if (d.is_invalid()) {
        m_cond = TRIG_EQ;
        return;
    }

    if (!d.is_scalar()) {
        throw ConfigFailureException("invalid condition value");
    }

    const string s = d.as<string>();

    if (s == "==" || s == "eq") {
        m_cond = TRIG_EQ;
    } else if (s == "!=" || s == "ne") {
        m_cond = TRIG_NE;
    } else if (s == "<" || s == "lt") {
        m_cond = TRIG_LT;
    } else if (s == "<=" || s == "le") {
        m_cond = TRIG_LE;
    } else if (s == ">" || s == "ge") {
        m_cond = TRIG_GE;
    } else if (s == ">=" || s == "gt") {
        m_cond = TRIG_GT;
    } else if (s == "range") {
        m_cond = TRIG_RANGE;
    } else if (s == "always") {
        m_cond = TRIG_ALWAYS;
    } else {
        throw ConfigFailureException(string("invalid condition value `") + s + "'");
    }
}

template <class T>
void TypedSignalEvent<T>::reconfigure(PlatformDescription &d)
{
    configure_event_action(d["on-event"]);
    configure_cond(d["condition"]);

    if (m_cond == TRIG_RANGE) {
        if (!d.exists("range")) {
            throw ConfigFailureException("missing range");
        }

        if (!d["range"].is_vector()) {
            throw ConfigFailureException("range must be an array");
        }

        try {
            m_range = d["range"].as< vector<T> >();
        } catch (PlatformDescription::InvalidConversionException &e) {
            throw ConfigFailureException("invalid range value(s)");
        }
    } else {
        if (!d.exists("value")) {
            throw ConfigFailureException("missing value");
        }

        if (!d["value"].is_scalar()) {
            throw ConfigFailureException("invalid value");
        }

        try {
            m_value = d["value"].as<T>();
        } catch (PlatformDescription::InvalidConversionException &e) {
            throw ConfigFailureException("cannot convert value to the required type");
        }
    }
}

template <class T>
bool TypedSignalEvent<T>::check_condition()
{
    T cur_value = m_parent.get_value<T>();

    switch (m_cond) {
    case TRIG_ALWAYS:
        return true;
    case TRIG_EQ:
        return m_value == cur_value;
    case TRIG_NE:
        return m_value != cur_value;
    case TRIG_LT:
        return m_value < cur_value;
    case TRIG_LE:
        return m_value <= cur_value;
    case TRIG_GT:
        return m_value > cur_value;
    case TRIG_GE:
        return m_value >= cur_value;
    case TRIG_RANGE:
        return m_range[0] <= cur_value && m_range[1] >= cur_value;
    }

    return false;
}

SignalGenerator::Ptr BackendInstance::create_generator(PlatformDescription &d)
{
    if (!m_generator) {
        const std::string & name = SignalGenerator::get_unique_name();
        m_generator = SignalGenerator::Ptr(new SignalGenerator(*this, name, d));

        if (m_elaboration_done) {
            apply_generator(m_generator);
        }
    } else {
        m_generator->reconfigure(d);
    }

    return m_generator;
}

SignalEvent::Ptr BackendInstance::create_event(PlatformDescription &d,
                                               JsonConsoleClient::Ptr client,
                                               PauseRequestListener &l)
{
    const std::string & name = SignalEvent::get_unique_name();

    SignalEvent::Ptr ev;

    if (m_type == "bool") {
        ev = SignalEvent::Ptr(new TypedSignalEvent<bool>(*this, name, d, client, l));
    } else if (m_type == "double") {
        ev = SignalEvent::Ptr(new TypedSignalEvent<double>(*this, name, d, client, l));
    } else {
        assert(false);
    }

    m_events.insert(ev);

    if (m_elaboration_done) {
        apply_event(ev);
    }

    return ev;
}

void BackendInstance::delete_event(SignalEvent::Ptr ev)
{
    if (m_elaboration_done) {
        m_backend->unregister_listener(*ev);
    }

    ev->set_deleted();
    m_events.erase(ev);
}

bool BackendInstance::sanity_checks()
{
    if (!m_builder->comp_exists(Namespace::get(Namespace::COMPONENT), m_comp_name)) {
        set_failure(SignalGenerator::FAIL_COMP_NOT_FOUND);
        return false;
    }

    ComponentBase & stubed
        = m_builder->get_comp(Namespace::get(Namespace::COMPONENT), m_comp_name);


    if (!stubed.port_exists(m_port_name)) {
        set_failure(SignalGenerator::FAIL_PORT_NOT_FOUND);
        return false;
    }

    return true;
}

bool BackendInstance::create_backend()
{
    string backend_type("stub-");
    backend_type += m_type;

    if (!m_builder->get_config()
        .get_backend_manager().type_exists(backend_type)) {
        set_failure(SignalGenerator::FAIL_INVALID_TYPE);
        return false;
    }

    BackendManager::Factory fact;
    fact = m_builder->get_config()
        .get_backend_manager()
        .find_by_type(backend_type);

    if (!fact) {
        set_failure(SignalGenerator::FAIL_INTERNAL);
        return false;
    }

    const string comp_name =
        string("backend-autogen-")
        + m_comp_name + "-"
        + m_port_name;

    Parameters p = fact->get_params();
    ComponentBase *base_comp = fact->create(comp_name, p);
    assert(base_comp);

    m_builder->add_backend(base_comp);

    m_backend = dynamic_cast<StubBackendBase*>(base_comp);
    assert(m_backend);

    return true;
}

bool BackendInstance::bind_backend()
{
    ComponentBase & stubed
        = m_builder->get_comp(Namespace::get(Namespace::COMPONENT), m_comp_name);

    if(!stubed.get_port(m_port_name).connect(m_backend->get_port("port"),
                                             PlatformDescription::INVALID_DESCRIPTION)) {
        set_failure(SignalGenerator::FAIL_BINDING);
        return false;
    }

    return true;
}

void BackendInstance::apply_generator(SignalGenerator::Ptr gen)
{
    m_backend->reconfigure(gen->get_description());
    gen->set_created();
}

void BackendInstance::apply_event(SignalEvent::Ptr event)
{
    m_backend->register_listener(*event);
    event->set_created();
}

void BackendInstance::apply_elements()
{
    if (m_generator) {
        apply_generator(m_generator);
    }

    for (auto &event: m_events) {
        apply_event(event);
    }
}

void BackendInstance::set_failure(SignalElement::FailureReason r)
{
    m_status = SignalElement::STA_FAILURE;
    m_failure_reason = r;

    if (m_generator) {
        m_generator->set_failure(r);
    }

    for (auto &ev : m_events) {
        ev->set_failure(r);
    }
}

void BackendInstance::elaborate(PlatformBuilder &builder)
{
    m_builder = &builder;

    if (!sanity_checks()) {
        return;
    }

    if (!create_backend()) {
        return;
    }

    m_status = SignalElement::STA_CREATED;

    if (!bind_backend()) {
        return;
    }

    apply_elements();

    m_elaboration_done = true;
}

void BackendInstance::reconfigure(PlatformDescription &d)
{
    if (m_backend) {
        m_backend->reconfigure(d);
    }
}
