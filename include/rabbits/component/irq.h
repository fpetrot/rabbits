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

#ifndef _UTILS_COMPONENT_IRQ_H
#define _UTILS_COMPONENT_IRQ_H

#include <string>
#include <systemc>
#include <map>

#include "rabbits/rabbits_exception.h"

class IrqIn;
class IrqOut;

class Irq {
protected:
    std::string m_name;
    unsigned int m_idx;
    bool m_is_connected;

public:
    Irq() : m_is_connected(false) {}
    Irq(const std::string &name, unsigned int idx)
        : m_name(name), m_idx(idx), m_is_connected(false) {}
    explicit Irq(const std::string &name)
        : m_name(name), m_is_connected(false) {}

    virtual ~Irq() {}

    bool is_connected() { return m_is_connected; }
    void set_connected() { m_is_connected = true; }
    std::string name() const { return m_name; }
    void set_name(const std::string &name) { m_name = name; }

    unsigned int idx() const { return m_idx; }
    void set_idx(unsigned int idx) { m_idx = idx; }

    virtual sc_core::sc_signal<bool>* connect(IrqIn &) = 0;
    virtual sc_core::sc_signal<bool>* connect(IrqOut &) = 0;
};

class IrqIn : public Irq {
protected:
    sc_core::sc_in<bool> *m_port;

public:
    IrqIn() {}
    IrqIn(const std::string name, unsigned int idx, sc_core::sc_in<bool>& port)
        : Irq(name, idx), m_port(&port) {}
    IrqIn(const std::string name, sc_core::sc_in<bool>& port)
        : Irq(name), m_port(&port) {}
    IrqIn(const std::string name, unsigned int idx)
        : Irq(name, idx) {}
    explicit IrqIn(const std::string name)
        : Irq(name) {}

    virtual ~IrqIn() {}

    virtual sc_core::sc_in<bool>& get_port() { return *m_port; }

    virtual sc_core::sc_signal<bool>* connect(IrqIn &in);
    virtual sc_core::sc_signal<bool>* connect(IrqOut &out);
};

class IrqOut : public Irq {
protected:
    sc_core::sc_out<bool> *m_port;

public:
    IrqOut() {}
    IrqOut(const std::string name, unsigned int idx, sc_core::sc_out<bool>& port)
        : Irq(name, idx), m_port(&port) {}
    IrqOut(const std::string name, sc_core::sc_out<bool>& port)
        : Irq(name), m_port(&port) {}
    IrqOut(const std::string name, unsigned int idx)
        : Irq(name, idx) {}
    explicit IrqOut(const std::string name)
        : Irq(name) {}

    virtual ~IrqOut() {}

    virtual sc_core::sc_out<bool>& get_port() { return *m_port; }

    virtual sc_core::sc_signal<bool>* connect(IrqIn &in);
    virtual sc_core::sc_signal<bool>* connect(IrqOut &out);
};


inline sc_core::sc_signal<bool>* IrqIn::connect(IrqIn &in) {
    (*m_port)(*(in.m_port));
    return NULL;
}

inline sc_core::sc_signal<bool>* IrqIn::connect(IrqOut &out) {
    sc_core::sc_signal<bool> *sig = new sc_core::sc_signal<bool>;

    sc_core::sc_in<bool> &p_in = this->get_port();
    sc_core::sc_out<bool> &p_out = out.get_port();

    p_in(*sig);
    p_out(*sig);

    set_connected();
    out.set_connected();

    return sig;
}

inline sc_core::sc_signal<bool>* IrqOut::connect(IrqIn &in) {
    sc_core::sc_signal<bool> *sig = new sc_core::sc_signal<bool>;

    sc_core::sc_in<bool> &p_in = in.get_port();
    sc_core::sc_out<bool> &p_out = this->get_port();

    p_in(*sig);
    p_out(*sig);

    set_connected();
    in.set_connected();

    return sig;
}

inline sc_core::sc_signal<bool>* IrqOut::connect(IrqOut &out) {
    (*m_port)(*(out.m_port));
    return NULL;
}



class IRQNotFoundException : public RabbitsException {
protected:
    std::string build_what(std::string id) { return "irq `" + id + "` not found"; }
    std::string build_what(unsigned int idx) {
        std::stringstream ss;
        ss << "irq index " << idx << " not found"; 
        return ss.str();
    }
public:
    explicit IRQNotFoundException(const std::string & id) : RabbitsException(build_what(id)) {}
    explicit IRQNotFoundException(int idx) : RabbitsException(build_what(idx)) {}
    virtual ~IRQNotFoundException() throw() {}
};


template <typename IRQ>
class IrqPool {
    std::map<std::string, IRQ*> m_irqs;
    std::vector<IRQ*> m_by_idx;

public:
    IrqPool() {}
    virtual ~IrqPool() {}

    void add(IRQ *irq) {
        m_irqs[irq->name()] = irq; 
        m_irqs[irq->name()]->set_idx(m_by_idx.size());
        m_by_idx.push_back(m_irqs[irq->name()]);
    }

    bool exists(std::string id) {
        return m_irqs.find(id) != m_irqs.end();
    }

    bool exists(unsigned int idx) {
        return m_by_idx.size() > idx;
    }

    IRQ& operator[] (const std::string id) {
        if (!exists(id)) {
            throw IRQNotFoundException(id);
        }
        return *m_irqs[id];
    }

    IRQ& operator[] (unsigned int idx) {
        if (!exists(idx)) {
            throw IRQNotFoundException(idx);
        }
        return *(m_by_idx[idx]);
    }

    typedef typename std::map<std::string, IRQ*>::iterator iterator;
    typedef typename std::map<std::string, IRQ*>::const_iterator const_iterator;

    iterator begin() { return m_irqs.begin(); }
    iterator end() { return m_irqs.end(); }
    const_iterator begin() const { return m_irqs.begin(); }
    const_iterator end() const { return m_irqs.end(); }
};


class HasIrqInIface {
public:
    virtual IrqIn& get_irq_in(std::string id) = 0;
    virtual IrqIn& get_irq_in(unsigned int idx) = 0;

    virtual bool irq_in_exists(std::string id) = 0;
    virtual bool irq_in_exists(unsigned int idx) = 0;

    typedef std::map<std::string, IrqIn*>::iterator iterator;
    typedef std::map<std::string, IrqIn*>::const_iterator const_iterator;

    virtual iterator irqs_in_begin() = 0;
    virtual iterator irqs_in_end() = 0;
    virtual const_iterator irqs_in_begin() const = 0;
    virtual const_iterator irqs_in_end() const = 0;
};

class HasIrqOutIface {
public:
    virtual IrqOut& get_irq_out(std::string id) = 0;
    virtual IrqOut& get_irq_out(unsigned int idx) = 0;

    virtual bool irq_out_exists(std::string id) = 0;
    virtual bool irq_out_exists(unsigned int idx) = 0;

    typedef std::map<std::string, IrqOut*>::iterator iterator;
    typedef std::map<std::string, IrqOut*>::const_iterator const_iterator;

    virtual iterator irqs_out_begin() = 0;
    virtual iterator irqs_out_end() = 0;
    virtual const_iterator irqs_out_begin() const = 0;
    virtual const_iterator irqs_out_end() const = 0;
};

#endif
