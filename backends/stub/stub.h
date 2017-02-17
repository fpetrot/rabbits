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
#include <vector>

#include <rabbits/component/component.h>
#include <rabbits/component/port/inout.h>

template <class T>
class Generator {
public:
    virtual sc_core::sc_time operator() (InOutPort<T> &p) = 0;
};

template <class T>
class GeneratorFactory {
protected:
    template <class PARAM>
    PARAM get_param(const Parameters &p, const char * name, const PARAM & default_val)
    {
        return default_val;
    }

public:
    virtual Generator<T> * create(const Parameters &p) = 0;
};

template <class T>
class SequenceGenerator : public Generator<T>
{
protected:
    std::vector<T> m_sequence;
    typename std::vector<T>::iterator m_it;

    bool m_loop = true;
    sc_core::sc_time m_sampling_time;

public:
    SequenceGenerator()
    {
        m_it = m_sequence.end();
    }

    void add(const T &v) { m_sequence.push_back(v); m_it = m_sequence.begin(); }

    void add(const std::vector<T> &v) {
        m_sequence.insert(m_sequence.end(), v.begin(), v.end());
        m_it = m_sequence.begin();
    }

    void clear() { m_sequence.clear(); m_it = m_sequence.end(); }
    void set_loop(bool loop) { m_loop = loop; }
    void set_sampling_period(sc_core::sc_time period) { m_sampling_time = period; }

    sc_core::sc_time operator() (InOutPort<T> &p)
    {
        if (m_sequence.empty()) {
            return sc_core::SC_ZERO_TIME;
        }

        if (m_it == m_sequence.end()) {
            if (m_loop) {
                m_it = m_sequence.begin();
            } else {
                return sc_core::SC_ZERO_TIME;
            }
        }

        p.sc_p = *(m_it++);
        return m_sampling_time;
    }
};

template <class T>
class SequenceGeneratorFactory : public GeneratorFactory<T>
{
public:
    Generator<T> * create(const Parameters &p)
    {
        SequenceGenerator<T> *gen = new SequenceGenerator<T>;

        gen->set_loop(p["sequence-repeat"].as<bool>());
        gen->set_sampling_period(p["sequence-sampling"].as<sc_core::sc_time>());
        gen->add(p["sequence"].as<std::vector<T> >());
        return gen;
    }
};

template <class T>
class StubBackend : public Component {
protected:
    std::map< std::string, GeneratorFactory<T>* > m_gen_factories;
    Generator<T> * m_cur_generator = nullptr;

    /* External (outside SystemC scope) event sampling period */
    sc_core::sc_time m_ext_ev_sampling;

    void register_generator_fact(const std::string &name, GeneratorFactory<T> *fact)
    {
        m_gen_factories[name] = fact;
    }

    void generator_thread();

    void set_generator(const Parameters &p)
    {
        const std::string gen = p["generator"].as<std::string>();

        if (gen == "none") {
            return;
        }

        if (m_gen_factories.find(gen) == m_gen_factories.end()) {
            MLOG(APP, WRN) << "Unknown generator " << gen << "\n";
            return;
        }

        m_cur_generator = m_gen_factories[gen]->create(p);
    }

public:
    SC_HAS_PROCESS(StubBackend);

    StubBackend(sc_core::sc_module_name n, const Parameters &p, ConfigManager &c)
        : Component(n, p, c), p_port("port")
    {
        register_generator_fact("sequence", new SequenceGeneratorFactory<T>);

        m_ext_ev_sampling = p["external-ev-sampling"].as<sc_core::sc_time>();

        set_generator(p);

        SC_THREAD(generator_thread);
    }

    virtual ~StubBackend() {}

    InOutPort<T> p_port;
};

template <class T>
void StubBackend<T>::generator_thread()
{
    for(;;) {
#if 0
        sc_core::wait(m_ext_ev_sampling);
#endif

        if (m_cur_generator) {
            sc_core::sc_time t = (*m_cur_generator)(p_port);

            if (t == sc_core::SC_ZERO_TIME) {
                return;
            }

            sc_core::wait(t);
        } else {
            return;
        }
    }
}
