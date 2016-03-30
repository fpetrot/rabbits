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
 * @file irq.h
 * @brief Irq, IrqIn, IrqOut and IrqPool classes declaration.
 */

#ifndef _UTILS_COMPONENT_IRQ_H
#define _UTILS_COMPONENT_IRQ_H

#include <string>
#include <systemc>
#include <map>

#include "rabbits/rabbits_exception.h"

class IrqIn;
class IrqOut;

/**
 * @brief An generic IRQ line.
 *
 * This class represents an generic IRQ line for a component (the direction is undefined).
 * It has a name and an index.
 * It can be connected to an IRQ line of another component or directly to a sc_signal<bool>.
 *
 * @see IrqIn
 * @see IrqOut
 */
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

    /**
     * @brief Return true if the IRQ line is connected.
     *
     * @return true if the IRQ line is connected, false otherwise.
     */
    bool is_connected() { return m_is_connected; }

    /**
     * @brief Set the IRQ in the connected state.
     */
    void set_connected() { m_is_connected = true; }

    /**
     * @brief Return the name of the IRQ line.
     *
     * @return the name of the IRQ line.
     */
    std::string name() const { return m_name; }

    /**
     * @brief Set the name of the IRQ line.
     *
     * @param[in] name The name of the IRQ line.
     */
    void set_name(const std::string &name) { m_name = name; }

    /**
     * @brief Return the index of the IRQ line.
     *
     * @return the index of the IRQ line.
     */
    unsigned int idx() const { return m_idx; }

    /**
     * @brief Set the index of the IRQ line.
     *
     * @param[in] idx The index of the IRQ line.
     */
    void set_idx(unsigned int idx) { m_idx = idx; }

    /**
     * @brief Connect the IRQ line to an input IRQ line.
     *
     * This method allows to connect the IRQ line to another input IRQ line.
     * When a sc_signal is needed to connect the two, it is dynamically
     * allocated and returned by this method. It is the responsibility of the
     * caller to handle the destruction of this sc_signal.
     * When no signal is needed, NULL is returned.
     *
     * Connecting two IRQ lines together set them in the connected state.
     *
     * @param[in,out] The input IRQ line to connect to.
     *
     * @return A pointer on the allocated sc_signal, NULL if no allocation was needed.
     */
    virtual sc_core::sc_signal<bool>* connect(IrqIn &) = 0;

    /**
     * @brief Connect the IRQ line to an output IRQ line.
     *
     * This method allows to connect the IRQ line to another output IRQ line.
     * When a sc_signal is needed to connect the two, it is dynamically
     * allocated and returned by this method. It is the responsibility of the
     * caller to handle the destruction of this sc_signal.
     * When no signal is needed, NULL is returned.
     *
     * Connecting two IRQ lines together set them in the connected state.
     *
     * @param[in,out] The output IRQ line to connect to.
     *
     * @return A pointer on the allocated sc_signal, NULL if no allocation was needed.
     */
    virtual sc_core::sc_signal<bool>* connect(IrqOut &) = 0;

    /**
     * @brief Connect the IRQ line to a sc_signal.
     *
     * This method will set the IRQ line to the connected state.
     *
     * @param[in] sig The sc_signal to connect to.
     */
    virtual void connect(sc_core::sc_signal<bool> &sig) = 0;
};

/**
 * @brief An input IRQ line.
 *
 * This class represents an input IRQ line for a component. It has a name, an
 * index and is associated to input port (sc_in<bool>) of the component.
 * It can be connected to an IRQ line of another component or directly to a sc_signal<bool>.
 */
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

    /**
     * @brief Return the sc_in<bool> port associated to this IRQ line.
     *
     * @return the sc_in<bool> associated to this IRQ line.
     */
    virtual sc_core::sc_in<bool>& get_port() { return *m_port; }

    virtual sc_core::sc_signal<bool>* connect(IrqIn &in);
    virtual sc_core::sc_signal<bool>* connect(IrqOut &out);

    virtual void connect(sc_core::sc_signal<bool> &sig);
};

/**
 * @brief An output IRQ line.
 *
 * This class represents an output IRQ line for a component. It has a name, an
 * index and is associated to input port (sc_out<bool>) of the component.
 * It can be connected to an IRQ line of another component or directly to a sc_signal<bool>.
 */
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

    /**
     * @brief Return the sc_out<bool> port associated to this IRQ line.
     *
     * @return the sc_out<bool> associated to this IRQ line.
     */
    virtual sc_core::sc_out<bool>& get_port() { return *m_port; }

    virtual sc_core::sc_signal<bool>* connect(IrqIn &in);
    virtual sc_core::sc_signal<bool>* connect(IrqOut &out);

    virtual void connect(sc_core::sc_signal<bool> &sig);
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

inline void IrqIn::connect(sc_core::sc_signal<bool> &sig) {
    sc_core::sc_in<bool> &p_in = this->get_port();
    p_in(sig);
    set_connected();
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

inline void IrqOut::connect(sc_core::sc_signal<bool> &sig) {
    sc_core::sc_out<bool> &p_out = this->get_port();
    p_out(sig);
    set_connected();
}



/**
 * @brief Raised when the requested IRQ is not found.
 */
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


/**
 * @brief An IRQ pool used by the components.
 *
 * @tparam IRQ the type used in the pool.
 */
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


/**
 * @brief Input IRQ lines interface.
 *
 * This interface represents a component having input IRQ lines.
 * The IRQ lines are named and indexed from 0 to (n-1), n being the total
 * number of input IRQ lines.
 */
class HasIrqInIface {
public:
    /**
     * @brief Return the input IRQ line of the given name.
     *
     * @param[in] id The name of the IRQ line.
     *
     * @return The input IRQ line.
     * @throw IRQNotFoundException if the IRQ does not exists.
     */
    virtual IrqIn& get_irq_in(std::string id) = 0;

    /**
     * @brief Return the input IRQ line of the given index.
     *
     * @param[in] idx The index of the IRQ line.
     *
     * @return The input IRQ line.
     * @throw IRQNotFoundException if the IRQ does not exists.
     */
    virtual IrqIn& get_irq_in(unsigned int idx) = 0;

    /**
     * @brief Return true if the IRQ line of the given name exists.
     *
     * @param[in] id The name of the IRQ line.
     *
     * @return true if the IRQ line of the given name exists, false otherwise.
     */
    virtual bool irq_in_exists(std::string id) = 0;

    /**
     * @brief Return true if the IRQ line of the given index exists.
     *
     * @param[in] id The index of the IRQ line.
     *
     * @return true if the IRQ line of the given index exists, false otherwise.
     */
    virtual bool irq_in_exists(unsigned int idx) = 0;

    typedef std::map<std::string, IrqIn*>::iterator iterator;
    typedef std::map<std::string, IrqIn*>::const_iterator const_iterator;

    /**
     * @brief Return an iterator to the first input IRQ line.
     *
     * @return an iterator to the first input IRQ line.
     */
    virtual iterator irqs_in_begin() = 0;

    /**
     * @brief Return an iterator to the <i>past-the-end</i> input IRQ line.
     *
     * @return an iterator to the <i>past-the-end</i> input IRQ line.
     */
    virtual iterator irqs_in_end() = 0;

    /**
     * @brief Return a constant iterator to the first input IRQ line.
     *
     * @return a constant iterator to the first input IRQ line.
     */
    virtual const_iterator irqs_in_begin() const = 0;

    /**
     * @brief Return a constant iterator to the <i>past-the-end</i> input IRQ line.
     *
     * @return an constant iterator to the <i>past-the-end</i> input IRQ line.
     */
    virtual const_iterator irqs_in_end() const = 0;
};

/**
 * @brief Output IRQ lines interface.
 *
 * This interface represents a component having output IRQ lines.
 * The IRQ lines are named and indexed from 0 to (n-1), n being the total
 * number of output IRQ lines.
 */
class HasIrqOutIface {
public:
    /**
     * @brief Return the output IRQ line of the given name.
     *
     * @param[in] id The name of the IRQ line.
     *
     * @return The output IRQ line.
     * @throw IRQNotFoundException if the IRQ does not exists.
     */
    virtual IrqOut& get_irq_out(std::string id) = 0;

    /**
     * @brief Return the output IRQ line of the given index.
     *
     * @param[in] idx The index of the IRQ line.
     *
     * @return The output IRQ line.
     * @throw IRQNotFoundException if the IRQ does not exists.
     */
    virtual IrqOut& get_irq_out(unsigned int idx) = 0;

    /**
     * @brief Return true if the IRQ line of the given name exists.
     *
     * @param[in] id The name of the IRQ line.
     *
     * @return true if the IRQ line of the given name exists, false otherwise.
     */
    virtual bool irq_out_exists(std::string id) = 0;

    /**
     * @brief Return true if the IRQ line of the given index exists.
     *
     * @param[in] id The index of the IRQ line.
     *
     * @return true if the IRQ line of the given index exists, false otherwise.
     */
    virtual bool irq_out_exists(unsigned int idx) = 0;

    typedef std::map<std::string, IrqOut*>::iterator iterator;
    typedef std::map<std::string, IrqOut*>::const_iterator const_iterator;

    /**
     * @brief Return an iterator to the first output IRQ line.
     *
     * @return an iterator to the first output IRQ line.
     */
    virtual iterator irqs_out_begin() = 0;

    /**
     * @brief Return an iterator to the <i>past-the-end</i> output IRQ line.
     *
     * @return an iterator to the <i>past-the-end</i> output IRQ line.
     */
    virtual iterator irqs_out_end() = 0;

    /**
     * @brief Return a constant iterator to the first output IRQ line.
     *
     * @return a constant iterator to the first output IRQ line.
     */
    virtual const_iterator irqs_out_begin() const = 0;

    /**
     * @brief Return a constant iterator to the <i>past-the-end</i> output IRQ line.
     *
     * @return an constant iterator to the <i>past-the-end</i> output IRQ line.
     */
    virtual const_iterator irqs_out_end() const = 0;
};

#endif
