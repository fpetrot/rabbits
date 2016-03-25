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

#ifndef _COMPONENTS_RASPBERRY_BUS_H
#define _COMPONENTS_RASPBERRY_BUS_H

#include <sstream>
#include <tlm>

#include "rabbits/component/component.h"
#include "rabbits/rabbits_exception.h"

class Bus : public Component, public BusIface {
public:
    Bus(sc_core::sc_module_name name, ComponentParameters& params) :
        Component(name, params) {}
    virtual ~Bus() {}
};

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

class BusSlaveIfaceBase : public tlm::tlm_bw_transport_if<> {
public:
    virtual ~BusSlaveIfaceBase() {}

    template <unsigned int BUSWIDTH>
    tlm::tlm_target_socket<BUSWIDTH>& get_socket() {
        BusSlaveIface<BUSWIDTH> *bus = dynamic_cast<BusSlaveIface<BUSWIDTH>* >(this);
        if (bus == NULL) {
            throw WrongBusSizeException(BUSWIDTH);
        }
        return bus->get_socket();
    }
};

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
    BusSlaveIface(SlaveIface &slave) : m_socket(NULL), m_free_socket(false), m_slave(&slave) {}
    BusSlaveIface(tlm::tlm_target_socket<BUSWIDTH> &s) : m_socket(&s), m_free_socket(false), m_slave(NULL) {}

    virtual ~BusSlaveIface() {
        if (m_free_socket) {
            delete m_socket;
        }
    }

    tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& t) {
        if (m_socket == NULL) {
            abort();
        }

        return (*m_socket)->nb_transport_bw(trans, phase, t);
    }

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
        if (m_socket == NULL) {
            abort();
        }

        (*m_socket)->invalidate_direct_mem_ptr(start_range, end_range);
    }

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

class BusMasterIfaceBase : public tlm::tlm_fw_transport_if<> {
public:
    virtual ~BusMasterIfaceBase() {}

    template <unsigned int BUSWIDTH>
    tlm::tlm_initiator_socket<BUSWIDTH>& get_socket() {
        BusMasterIface<BUSWIDTH> *bus = dynamic_cast<BusMasterIface<BUSWIDTH>* >(this);
        if (bus == NULL) {
            throw WrongBusSizeException(BUSWIDTH);
        }
        return bus->get_socket();
    }
};

template <unsigned int BUSWIDTH>
class BusMasterIface : public BusMasterIfaceBase {
protected:
    tlm::tlm_initiator_socket<BUSWIDTH>* m_socket;
    bool m_free_socket;
    MasterIface *m_master;

public:
    BusMasterIface(MasterIface &master) : m_socket(NULL), m_free_socket(false), m_master(&master) {}
    BusMasterIface(tlm::tlm_target_socket<BUSWIDTH> &m) : m_socket(&m), m_free_socket(false), m_master(NULL) {}

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
