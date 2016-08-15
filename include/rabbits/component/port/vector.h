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

#ifndef _RABBITS_COMPONENT_PORT_VECTOR_H
#define _RABBITS_COMPONENT_PORT_VECTOR_H

#include <vector>
#include <sstream>
#include <functional>
#include <memory>
#include <iterator>

#include "rabbits/component/port.h"
#include "rabbits/component/port/in.h"
#include "rabbits/component/port/out.h"
#include "rabbits/component/port/inout.h"
#include "rabbits/component/connection_strategy/vector.h"

template <typename T>
class VectorPortBase : public Port {
public:
    typedef std::vector< std::unique_ptr<T> > Container;

    class iterator : public std::iterator<std::input_iterator_tag, T> {
    private:
        Container &m_vec;
        typename Container::iterator m_cur;


    public:
        iterator(VectorPortBase<T> &vec) : m_vec(vec.m_ports), m_cur(m_vec.begin()) {}
        iterator(const iterator &it) : m_vec(it.m_vec), m_cur(it.m_cur) {}

        iterator& operator++() { ++m_cur; return *this; }
        iterator operator++(int) {iterator tmp(*this); operator++(); return tmp; }

        bool operator==(const iterator& it) { return m_cur == it.m_cur; }
        bool operator!=(const iterator& it) { return m_cur != it.m_cur; }

        T& operator*() { return **m_cur; }

        iterator end() { iterator tmp(*this); tmp.m_cur = m_vec.end(); return tmp;}
    };

    typedef typename Container::size_type size_type;

protected:
    Container m_ports;

    VectorCS<T> m_cs;

    void declare_parent() {
        if (!m_ports.size()) {
            return;
        }

        Port *child_port = dynamic_cast<Port*>(m_ports[0].get());

        if (child_port) {
            Port::declare_parent(child_port->get_parent());
        }

        add_connection_strategy(m_cs);
    }

public:
    VectorPortBase(const std::string &name, unsigned int size)
        : Port(name)
        , m_ports(size)
        , m_cs(*this)
    {
        int i = 0;
        for (std::unique_ptr<T> & p : m_ports) {
            std::stringstream ss;

            ss << name << i++;
            p.reset(new T(ss.str()));
        }

        declare_parent();
    }

    VectorPortBase(const std::string &name, unsigned int size,
                   std::function<T*(const std::string&)> generator)
        : Port(name)
        , m_ports(size)
        , m_cs(*this)
    {
        int i = 0;
        for (std::unique_ptr<T> & p : m_ports) {
            std::stringstream ss;

            ss << name << i++;
            p.reset(generator(ss.str()));
        }

        declare_parent();
    }

    VectorPortBase(const std::string &name, unsigned int size,
                   std::function<T*(const std::string&, int)> generator)
        : Port(name)
        , m_ports(size)
        , m_cs(*this)
    {
        int i = 0;
        for (std::unique_ptr<T> & p : m_ports) {
            p.reset(generator(name, i++));
        }

        declare_parent();
    }

    virtual ~VectorPortBase() {}

    iterator begin() { return iterator(*this); }
    iterator end() { return iterator(*this).end(); }

    T& operator[] (size_type n) {
        return *(m_ports[n]);
    }

    size_type size() const {
        return m_ports.size();
    }

};

template <typename T>
class VectorPort : public VectorPortBase<T>
{
public:
    using VectorPortBase<T>::VectorPortBase;
};

template <typename Tport>
class VectorPort< InPort<Tport> > : public VectorPortBase< InPort<Tport> >
{
protected:
    using VectorPortBase< InPort<Tport> >::m_ports;

public:
    using VectorPortBase< InPort<Tport> >::VectorPortBase;

    VectorPort< InPort<Tport> >(const std::string &name,
                                sc_core::sc_vector< sc_core::sc_in<Tport> > &sub_ports)
        : VectorPortBase< InPort<Tport> >(name, sub_ports.size())
    {
        int i = 0;
        for (auto &sub_p : sub_ports) {
        sub_p(m_ports[i]->sc_p);
            i++;
        }
    }
};


template <typename Tport>
class VectorPort< OutPort<Tport> > : public VectorPortBase< OutPort<Tport> >
{
protected:
    using VectorPortBase< OutPort<Tport> >::m_ports;

public:
    using VectorPortBase< OutPort<Tport> >::VectorPortBase;

    VectorPort< OutPort<Tport> >(const std::string &name,
                                 sc_core::sc_vector< sc_core::sc_out<Tport> > &sub_ports)
        : VectorPortBase< OutPort<Tport> >(name, sub_ports.size())
    {
        int i = 0;
        for (auto &sub_p : sub_ports) {
            sub_p(m_ports[i]->sc_p);
            i++;
        }
    }
};


template <typename Tport>
class VectorPort< InOutPort<Tport> > : public VectorPortBase< InOutPort<Tport> >
{
protected:
    using VectorPortBase< InOutPort<Tport> >::m_ports;

public:
    using VectorPortBase< InOutPort<Tport> >::VectorPortBase;

    VectorPort< InOutPort<Tport> >(const std::string &name,
                                   sc_core::sc_vector< sc_core::sc_inout<Tport> > &sub_ports)
        : VectorPortBase< InOutPort<Tport> >(name, sub_ports.size())
    {
        int i = 0;
        for (auto &sub_p : sub_ports) {
            sub_p(m_ports[i]->sc_p);
            i++;
        }
    }
};

#include "rabbits/component/connection_strategy/vector_impl.h"

#endif
