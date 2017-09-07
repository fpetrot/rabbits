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

#include <map>
#include <set>

#include <rabbits/component/component.h>
#include <rabbits/component/port/inout.h>

#include "generator.h"

class StubEventListener {
public:
    virtual void stub_event() = 0;
};

template <class T>
class StubBackend;

class StubBackendBase : public Component {
private:
    std::set<StubEventListener*> m_listeners;

protected:
    void signal_event() const
    {
        for (auto *l : m_listeners) {
            l->stub_event();
        }
    }

    virtual void reconfigure(const Parameters &p) = 0;

public:
    StubBackendBase(sc_core::sc_module_name n, const Parameters &p, ConfigManager &c)
        : Component(n, p, c) {}

    void register_listener(StubEventListener &l)
    {
        m_listeners.insert(&l);
    }

    void unregister_listener(StubEventListener &l)
    {
        m_listeners.erase(&l);
    }

    void reconfigure(PlatformDescription &d)
    {
        Parameters p = m_params;
        p.fill_from_description(d);
        reconfigure(p);
    }

    template <class T>
    T get_value() const
    {
        const StubBackend<T> & child = dynamic_cast<const StubBackend<T>&>(*this);
        return child.get_value();
    }
};

template <class T>
class StubBackend : public StubBackendBase {
protected:
    std::map< std::string, GeneratorFactory<T>* > m_gen_factories;
    Generator<T> * m_cur_generator = nullptr;
    Generator<T> * m_next_generator = nullptr;

    T m_sampled_value;

    /* External (outside SystemC scope) event sampling period */
    sc_core::sc_time m_ext_ev_sampling;

    /* Deadlines */
    sc_core::sc_time m_next_ext_ev_dl;
    sc_core::sc_time m_next_gen_dl;

    void register_generator_fact(const std::string &name, GeneratorFactory<T> *fact)
    {
        m_gen_factories[name] = fact;
    }

    void generator_thread();
    void event_method();

    void set_generator(const Parameters &p)
    {
        const std::string gen = p["generator"].as<std::string>();

        if (m_cur_generator != nullptr) {
            delete m_cur_generator;
            m_cur_generator = nullptr;
        }

        if (gen == "none") {
            return;
        }

        if (m_gen_factories.find(gen) == m_gen_factories.end()) {
            MLOG(APP, WRN) << "Unknown generator " << gen << "\n";
            return;
        }

        m_next_generator = m_gen_factories[gen]->create(p);
    }

    void destroy_generator()
    {
        delete m_cur_generator;
        m_cur_generator = nullptr;
    }

    void update_generator()
    {
        if (m_next_generator != nullptr) {
            destroy_generator();
            m_cur_generator = m_next_generator;
            m_next_generator = nullptr;
        }
    }

    void reconfigure(const Parameters &p)
    {
        MLOG(APP, DBG) << "Reconfiguring backend\n";
        set_generator(p);
    }

public:
    SC_HAS_PROCESS(StubBackend);

    StubBackend(sc_core::sc_module_name n, const Parameters &p, ConfigManager &c)
        : StubBackendBase(n, p, c), p_port("port")
    {
        register_generator_fact("sequence", new SequenceGeneratorFactory<T>);

        m_ext_ev_sampling = p["external-ev-sampling"].as<sc_core::sc_time>();

        set_generator(p);

        SC_THREAD(generator_thread);

        SC_METHOD(event_method);
        sensitive << p_port.sc_p;

        p_port.set_autoconnect_to(0);
    }

    virtual ~StubBackend() {}

    /**
     * @brief Return the current value of the port connected to the backend
     *
     * This method return the current value of the port connected to the backend.
     * This method is safe to be called outside of the SystemC simulation context.
     *
     * @return the current value of the port connected to the backend
     */
    T get_value() const
    {
        return m_sampled_value;
    }


    InOutPort<T> p_port;
};

template <class T>
void StubBackend<T>::generator_thread()
{
    for(;;) {
        sc_core::sc_time next_dl;

        update_generator();

        if (m_next_ext_ev_dl == sc_core::SC_ZERO_TIME) {
            m_next_ext_ev_dl = m_ext_ev_sampling;
        }

        if ((m_next_gen_dl == sc_core::SC_ZERO_TIME) && m_cur_generator) {
            sc_core::sc_time t = (*m_cur_generator)(p_port);

            if (t == sc_core::SC_ZERO_TIME) {
                destroy_generator();
            } else {
                m_next_gen_dl = t;
            }
        }

        if (m_cur_generator) {
            next_dl = std::min(m_next_gen_dl, m_next_ext_ev_dl);
            m_next_gen_dl -= next_dl;
        } else {
            next_dl = m_next_ext_ev_dl;
        }

        m_next_ext_ev_dl -= next_dl;
        sc_core::wait(next_dl);
    }
}

template <class T>
void StubBackend<T>::event_method()
{
    m_sampled_value = p_port.sc_p;
    signal_event();
}
