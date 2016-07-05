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
 * @file master.h
 * Master class declaration
 */

#ifndef _MASTER_DEVICE_H_
#define _MASTER_DEVICE_H_

#include <systemc>

#include "rabbits/logger.h"
#include "rabbits/component/component.h"
#include "rabbits/component/port/tlm_initiator.h"

/**
 * @brief Master (initiator) component on a bus.
 *
 * Represent a component that is connected as a master (a initiator) on a bus.
 */
template <unsigned int BUSWIDTH = 32>
class MasterTraits: public tlm::tlm_bw_transport_if<>
{
public:
    TlmInitiatorPort<BUSWIDTH> p_bus;

    MasterTraits() : p_bus("bus", *this) {}

    virtual ~MasterTraits() {}

    /**
     * @brief Emit a read request on the bus the master is connected to.
     *
     * @param[in] addr Address of the read request.
     * @param[in,out] data Array where read result must be written.
     * @param[in] len Length of the read request.
     */
    virtual void bus_read(uint64_t addr, uint8_t *data, unsigned int len)
    {
        p_bus.bus_read(addr, data, len);
    }

    /**
     * @brief Emit a write request on the bus the master is connected to.
     *
     * @param[in] addr Address of the write request.
     * @param[in] data Array containing the data of the write request.
     * @param[in] len Length of the write request.
     */
    virtual void bus_write(uint64_t addr, uint8_t *data, unsigned int len)
    {
        p_bus.bus_write(addr, data, len);
    }

    /* tlm::tlm_bw_transport_if */
    virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans,
                                               tlm::tlm_phase& phase,
                                               sc_core::sc_time& t)
    {
        LOG(SIM, ERR) << "Non-blocking transport not implemented\n";
        abort();
        return tlm::TLM_COMPLETED;
    }

    virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                           sc_dt::uint64 end_range)
    {
        LOG(SIM, ERR) << "DMI memory invalidation not implemented\n";
        abort();
    }
};


template <unsigned int BUSWIDTH = 32>
class Master : public Component, public MasterTraits<BUSWIDTH> {
public:
    explicit Master(sc_core::sc_module_name name) : Component(name, ComponentParameters()) {}
    Master(sc_core::sc_module_name name, ComponentParameters &params) : Component(name, params) {}

    virtual ~Master() {}
};
#endif
