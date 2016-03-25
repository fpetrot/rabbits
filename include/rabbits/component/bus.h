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
 * @file Bus and Bus interfaces class declaration.
 */

#ifndef _COMPONENTS_RASPBERRY_BUS_H
#define _COMPONENTS_RASPBERRY_BUS_H

#include <sstream>
#include <tlm>

#include "rabbits/component/component.h"
#include "rabbits/rabbits_exception.h"

/**
 * @brief Bus component.
 *
 * This class represents a bus component. A bus component can connect Masters
 * and Slaves together through BusMasterIface and BusSlaveIface interfaces, and
 * forward TLM requests.
 */
class Bus : public Component, public BusIface {
public:
    Bus(sc_core::sc_module_name name, ComponentParameters& params) :
        Component(name, params) {}
    virtual ~Bus() {}
};


/**
 * @brief Wrong bus size exception.
 *
 * Exception thrown when two components of different bus sizes are connected together
 */
class WrongBusSizeException : public RabbitsException {
protected:
    std::string build_what(unsigned int bits) {
        std::stringstream ss;
        ss << "The bus associated to this socket is not " << bits << " bits";
        return ss.str();
    }
public:
    explicit WrongBusSizeException(unsigned int bits) : RabbitsException(build_what(bits)) {}
    virtual ~WrongBusSizeException() throw() {}
};


template <unsigned int BUSWIDTH = 32>
class BusSlaveIface;


/**
 * @brief Slave bus interface, base class.
 *
 * Base class of the slave bus interface. This class is bus size agnostic and a
 * runtime check is performed to ensure bus size compatibility.
 */
class BusSlaveIfaceBase : public tlm::tlm_bw_transport_if<> {
public:
    virtual ~BusSlaveIfaceBase() {}


    /**
     * @brief Get the target TLM socket associated to the interface.
     *
     * @tparam BUSWIDTH Bus size of the socket.
     *
     * @return the TLM socket associated to the interface.
     * @throw WrongBusSizeException if BUSWIDTH does not match the socket bus size.
     */
    template <unsigned int BUSWIDTH>
    tlm::tlm_target_socket<BUSWIDTH>& get_socket() {
        BusSlaveIface<BUSWIDTH> *bus = dynamic_cast<BusSlaveIface<BUSWIDTH>* >(this);
        if (bus == NULL) {
            throw WrongBusSizeException(BUSWIDTH);
        }
        return bus->get_socket();
    }
};

/**
 * @brief Slave bus interface.
 *
 * Slave bus interface with fixed bus size. This class encapsulate the TLM
 * target socket and provide all the methods a target socket provides.
 *
 * @tparam BUSWIDTH bus size of the interface.
 */
template <unsigned int BUSWIDTH>
class BusSlaveIface : public BusSlaveIfaceBase {
protected:
    tlm::tlm_target_socket<BUSWIDTH> *m_socket;
    bool m_free_socket;
    SlaveIface *m_slave;

    void set_socket(tlm::tlm_target_socket<BUSWIDTH>& s) {
        m_socket = s;
    }

public:
    /**
     * @brief Construct a slave interface for a given slave.
     *
     * When using this constructor, the slave interface instance handles the TLM target socket itself.
     * It creates and binds it to the slave on first use (ie. when connected to a bus).
     *
     * @param slave The Slave associated to this SlaveIface.
     */
    BusSlaveIface(SlaveIface &slave) : m_socket(NULL), m_free_socket(false), m_slave(&slave) {}


    /**
     * @brief Construct a slave interface for a given TLM socket.
     *
     * When using this constructor, the slave interface assumes that the TLM
     * target socket is fully valid and bound, and does not handle its
     * lifetime.
     *
     * @param s The TLM target socket associated to this SlaveIface.
     */
    BusSlaveIface(tlm::tlm_target_socket<BUSWIDTH> &s) : m_socket(&s), m_free_socket(false), m_slave(NULL) {}

    virtual ~BusSlaveIface() {
        if (m_free_socket) {
            delete m_socket;
        }
    }

    /**
     * @brief TLM Non-blocking transport backward.
     *
     * See TLM2.0 documentation.
     *
     * @param trans TLM payload.
     * @param phase TLM phase.
     * @param t elapsed time.
     *
     * @return TLM transport status
     */
    tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& t) {
        if (m_socket == NULL) {
            abort();
        }

        return (*m_socket)->nb_transport_bw(trans, phase, t);
    }

    /**
     * @brief TLM DMI invalidation
     *
     * See TLM2.0 documentation
     *
     * @param start_range Start address
     * @param end_range End address
     */
    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
        if (m_socket == NULL) {
            abort();
        }

        (*m_socket)->invalidate_direct_mem_ptr(start_range, end_range);
    }

    /**
     * @brief Get the TLM socket associated to the interface
     *
     * @return the TLM target socket associated to the interface
     */
    tlm::tlm_target_socket<BUSWIDTH>& get_socket() {
        if (m_socket == NULL) {
            m_socket = new tlm::tlm_target_socket<BUSWIDTH>;
            m_socket->bind(*m_slave);
            m_free_socket = true;
        }

        return *m_socket;
    }
};


template <unsigned int BUSWIDTH = 32>
class BusMasterIface;

/**
 * @brief Master bus interface, base class.
 *
 * Base class of the master bus interface. This class is bus size agnostic and a
 * runtime check is performed to ensure bus size compatibility.
 */
class BusMasterIfaceBase : public tlm::tlm_fw_transport_if<> {
public:
    virtual ~BusMasterIfaceBase() {}

    /**
     * @brief Get the initiator TLM socket associated to the interface.
     *
     * @tparam BUSWIDTH Bus size of the socket.
     *
     * @return the TLM socket associated to the interface.
     * @throw WrongBusSizeException if BUSWIDTH does not match the socket bus size.
     */
    template <unsigned int BUSWIDTH>
    tlm::tlm_initiator_socket<BUSWIDTH>& get_socket() {
        BusMasterIface<BUSWIDTH> *bus = dynamic_cast<BusMasterIface<BUSWIDTH>* >(this);
        if (bus == NULL) {
            throw WrongBusSizeException(BUSWIDTH);
        }
        return bus->get_socket();
    }
};

/**
 * @brief Master bus interface.
 *
 * Master bus interface with fixed bus size. This class encapsulate the TLM
 * initiator socket and provide all the methods a initiator socket provides.
 *
 * @tparam BUSWIDTH bus size of the interface.
 */
template <unsigned int BUSWIDTH>
class BusMasterIface : public BusMasterIfaceBase {
protected:
    tlm::tlm_initiator_socket<BUSWIDTH>* m_socket;
    bool m_free_socket;
    MasterIface *m_master;

public:
    /**
     * @brief Construct a master interface for a given master.
     *
     * When using this constructor, the master interface instance handles the TLM initiator socket itself.
     * It creates and binds it to the master on first use (ie. when connected to a bus).
     *
     * @param master The Master associated to this MasterIface.
     */
    BusMasterIface(MasterIface &master) : m_socket(NULL), m_free_socket(false), m_master(&master) {}

    /**
     * @brief Construct a master interface for a given TLM socket.
     *
     * When using this constructor, the master interface assumes that the TLM
     * initiator socket is fully valid and bound, and does not handle its
     * lifetime.
     *
     * @param s The TLM initiator socket associated to this MasterIface.
     */
    BusMasterIface(tlm::tlm_initiator_socket<BUSWIDTH> &m) : m_socket(&m), m_free_socket(false), m_master(NULL) {}

    virtual ~BusMasterIface() {
        if (m_free_socket) {
            delete m_socket;
        }
    }

    void b_transport(tlm::tlm_generic_payload &pl, sc_core::sc_time &t) {
        (*m_socket)->b_transport(pl, t);
    }

    unsigned int transport_dbg(tlm::tlm_generic_payload &pl) {
        return (*m_socket)->transport_dbg(pl);
    }

    bool get_direct_mem_ptr(tlm::tlm_generic_payload &pl, tlm::tlm_dmi &dmi_data) {
        return (*m_socket)->get_direct_mem_ptr(pl, dmi_data);
    }

    tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans,
                                       tlm::tlm_phase& phase, sc_core::sc_time& t) {
        return (*m_socket)->nb_transport_fw(trans, phase, t);
    }

    /**
     * @brief Get the TLM socket associated to the interface
     *
     * @return the TLM initiator socket associated to the interface
     */
    tlm::tlm_initiator_socket<BUSWIDTH>& get_socket() {
        if (m_socket == NULL) {
            m_socket = new tlm::tlm_initiator_socket<BUSWIDTH>;
            m_socket->bind(*m_master);
            m_free_socket = true;
        }

        return *m_socket;
    }
};

#endif
