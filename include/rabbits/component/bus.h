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
class BusMasterIface;

class BusMasterIfaceBase {
public:
    virtual ~BusMasterIfaceBase() {}

    virtual void b_transport(tlm::tlm_generic_payload &pl, sc_core::sc_time &t) = 0;
    virtual unsigned int transport_dbg(tlm::tlm_generic_payload &pl) = 0;
    virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload &pl, tlm::tlm_dmi &dmi_data) = 0;

    template <unsigned int BUSWIDTH>
    tlm::tlm_initiator_socket<BUSWIDTH>& get_socket() {
        BusMasterIface<BUSWIDTH> *bus = dynamic_cast<BusMasterIface<BUSWIDTH> >(this);
        if (bus == NULL) {
            throw WrongBusSizeException(BUSWIDTH);
        }
        return bus->get_socket();
    }
};

template <unsigned int BUSWIDTH>
class BusMasterIface : public BusMasterIfaceBase {
protected:
    tlm::tlm_initiator_socket<BUSWIDTH> m_socket;

public:
    virtual ~BusMasterIface() {}

    virtual void b_transport(tlm::tlm_generic_payload &pl, sc_core::sc_time &t) {
        m_socket->b_transport(pl, t);
    }

    virtual unsigned int transport_dbg(tlm::tlm_generic_payload &pl) {
        return m_socket->transport_dbg(pl);
    }

    virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload &pl, tlm::tlm_dmi &dmi_data) {
        return m_socket->get_direct_mem_ptr(pl, dmi_data);
    }

    tlm::tlm_initiator_socket<BUSWIDTH>& get_socket() { return m_socket; }
};

#endif
