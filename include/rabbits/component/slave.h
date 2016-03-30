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
 * @file slave.h
 * @brief Slave class declaration
 */

#ifndef _SLAVE_DEVICE_H_
#define _SLAVE_DEVICE_H_

#include <systemc>
#include <tlm>

#include "rabbits/logger.h"

#include "rabbits/component/component.h"

/**
 * @brief Slave (target) component on a bus
 *
 * Represent a component that is connected as a slave (a target) on a bus.
 */
class Slave: public Component, public SlaveIface
{
protected:
    BusSlaveIfaceBase *m_bus_iface;

public:
    Slave(sc_core::sc_module_name name, const ComponentParameters &params);
    Slave(sc_core::sc_module_name name);
    virtual ~Slave();


    /**
     * @brief Callback method on bus read request.
     *
     * This method is called on bus read request targeted to the component.
     * The Slave class implementation calls sub-methods depending on the length of the request.
     * It can be overridden by the child class when special bus request handling is needed.
     *
     * @param[in] addr Address of the request.
     * @param[out] data Array where read result must be written.
     * @param[in] len Length requested for the read request.
     * @param[out] bErr To be set to true to signal a bus error.
     *
     * @see bus_cb_read_8
     * @see bus_cb_read_16
     * @see bus_cb_read_32
     */
    virtual void bus_cb_read(uint64_t addr, uint8_t *data, unsigned int len, bool &bErr) {
        switch (len) {
        case 1:
            bus_cb_read_8(addr, (uint8_t *)data, bErr);
            break;
        case 2:
            bus_cb_read_16(addr, (uint16_t *)data, bErr);
            break;
        case 4:
            bus_cb_read_32(addr, (uint32_t *)data, bErr);
            break;
        }
    }

    /**
     * @brief Callback method on 8-bit bus read request
     *
     * This method is called on a 8-bit bus read request targeted to the component.
     * The Slave class implementation signals a bus error. Children classes
     * must override it if they want to support 8-bit read requests.
     *
     * @param[in] addr Address of the request.
     * @param[out] data Array where read result must be written.
     * @param[out] bErr To be set to true to signal a bus error.
     *
     * @see bus_cb_read_16
     * @see bus_cb_read_32
     */
    virtual void bus_cb_read_8(uint64_t addr, uint8_t *value, bool &bErr) {
        bErr = true;
    }

    /**
     * @brief Callback method on 16-bit bus read request
     *
     * This method is called on a 16-bit bus read request targeted to the component.
     * The Slave class implementation signals a bus error. Children classes
     * must override it if they want to support 16-bit read requests.
     *
     * @param[in] addr Address of the request.
     * @param[out] data Array where read result must be written.
     * @param[out] bErr To be set to true to signal a bus error.
     *
     * @see bus_cb_read_8
     * @see bus_cb_read_32
     */
    virtual void bus_cb_read_16(uint64_t addr, uint16_t *value, bool &bErr) {
        bErr = true;
    }

    /**
     * @brief Callback method on 32-bit bus read request
     *
     * This method is called on a 32-bit bus read request targeted to the component.
     * The Slave class implementation signals a bus error. Children classes
     * must override it if they want to support 32-bit read requests.
     *
     * @param[in] addr Address of the request.
     * @param[out] data Array where read result must be written.
     * @param[out] bErr To be set to true to signal a bus error.
     *
     * @see bus_cb_read_8
     * @see bus_cb_read_16
     */
    virtual void bus_cb_read_32(uint64_t addr, uint32_t *value, bool &bErr) {
        bErr = true;
    }


    /**
     * @brief Callback method on bus write request.
     *
     * This method is called on bus write request targeted to the component.
     * The Slave class implementation calls sub-methods depending on the length of the request.
     * It can be overridden by the child class when special bus request handling is needed.
     *
     * @param[in] addr Address of the request.
     * @param[in] data Array containing the data of the write request.
     * @param[in] len Length requested for the write request.
     * @param[out] bErr To be set to true to signal a bus error.
     *
     * @see bus_cb_write_8
     * @see bus_cb_write_16
     * @see bus_cb_write_32
     */
    virtual void bus_cb_write(uint64_t addr, uint8_t *data, unsigned int len, bool &bErr) {
        switch (len) {
        case 1:
            bus_cb_write_8(addr, (uint8_t *)data, bErr);
            break;
        case 2:
            bus_cb_write_16(addr, (uint16_t *)data, bErr);
            break;
        case 4:
            bus_cb_write_32(addr, (uint32_t *)data, bErr);
            break;
        }
    }

    /**
     * @brief Callback method on 8-bit bus write request
     *
     * This method is called on a 8-bit bus write request targeted to the component.
     * The Slave class implementation signals a bus error. Children classes
     * must override it if they want to support 8-bit write requests.
     *
     * @param[in] addr Address of the request.
     * @param[in] data Array containing the data of the write request.
     * @param[out] bErr To be set to true to signal a bus error.
     *
     * @see bus_cb_write_16
     * @see bus_cb_write_32
     */
    virtual void bus_cb_write_8(uint64_t addr, uint8_t *value, bool &bErr) {
        bErr = true;
    }

    /**
     * @brief Callback method on 16-bit bus write request
     *
     * This method is called on a 16-bit bus write request targeted to the component.
     * The Slave class implementation signals a bus error. Children classes
     * must override it if they want to support 16-bit write requests.
     *
     * @param[in] addr Address of the request.
     * @param[in] data Array containing the data of the write request.
     * @param[out] bErr To be set to true to signal a bus error.
     *
     * @see bus_cb_write_8
     * @see bus_cb_write_32
     */
    virtual void bus_cb_write_16(uint64_t addr, uint16_t *value, bool &bErr) {
        bErr = true;
    }

    /**
     * @brief Callback method on 32-bit bus write request
     *
     * This method is called on a 32-bit bus write request targeted to the component.
     * The Slave class implementation signals a bus error. Children classes
     * must override it if they want to support 32-bit write requests.
     *
     * @param[in] addr Address of the request.
     * @param[in] data Array containing the data of the write request.
     * @param[out] bErr To be set to true to signal a bus error.
     *
     * @see bus_cb_write_8
     * @see bus_cb_write_16
     */
    virtual void bus_cb_write_32(uint64_t addr, uint32_t *value, bool &bErr) {
        bErr = true;
    }

    /**
     * @brief Callback method on debug read request.
     *
     * This method is called on a bus debug request. This kind of request is
     * supposed to have no side effect on the component (such as time
     * consumption) and is used for debugging purpose.
     *
     * The Slave class implementation always returns 0.
     *
     * @param[in] addr Address of the request.
     * @param[out] buf Array where read result must be written.
     * @param[in] size Length of the request.
     *
     * @return The number of bytes effectively read.
     */
    virtual uint64_t debug_read(uint64_t addr, uint8_t* buf, uint64_t size) {
        return 0;
    }

    /**
     * @brief Callback method on debug write request.
     *
     * This method is called on a bus debug request. This kind of request is
     * supposed to have no side effect on the component (such as time
     * consumption) and is used for debugging purpose.
     *
     * The Slave class implementation always returns 0.
     *
     * @param[in] addr Address of the request.
     * @param[in] buf Array containing the data of the write request.
     * @param[in] size Length of the request.
     *
     * @return The number of bytes effectively written.
     */
    virtual uint64_t debug_write(uint64_t addr, const uint8_t* buf, uint64_t size) {
        return 0;
    }

    /**
     * @brief Callback method on direct memory access request
     *
     * This method is called when an initiator emits a TLM2.0 DMI (direct
     * memory interface) request directed to this component. Thes Slave class
     * implementation always returns false to signal that DMI is not supported.
     *
     * @param[in] trans TLM2.0 payload
     * @param[out] dmi_data TLM2.0 DMI data
     *
     * @return true if DMI is supported, false otherwise
     */
    virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                                    tlm::tlm_dmi& dmi_data)
    {
        return false;
    }

    virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans,
                                               tlm::tlm_phase& phase,
                                               sc_core::sc_time& t)
    {
        ERR_PRINTF("Non-blocking transport not implemented\n");
        abort();
        return tlm::TLM_COMPLETED;
    }

    virtual void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);
    virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans);

    /* SlaveIface */
    virtual ComponentBase& get_component() { return *this; }
    virtual void set_bus_iface(BusSlaveIfaceBase *iface) { m_bus_iface = iface; }
    virtual bool bus_iface_is_set() { return m_bus_iface != NULL; }
    virtual BusSlaveIfaceBase & get_bus_iface() { return *m_bus_iface; }

};


#endif
