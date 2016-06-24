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

#ifndef _RABBITS_DATATYPES_TLM_H
#define _RABBITS_DATATYPES_TLM_H

#include <systemc>
#include <tlm>

#include <vector>

#include "rabbits/datatypes/address_range.h"

template <unsigned int BUSWIDTH = 32>
class TlmSocketBase {
public:
    typedef tlm::tlm_base_target_socket_b<BUSWIDTH,
                                          tlm::tlm_fw_transport_if<>,
                                          tlm::tlm_bw_transport_if<> > Target;

    typedef tlm::tlm_base_initiator_socket_b<BUSWIDTH,
                                             tlm::tlm_fw_transport_if<>,
                                             tlm::tlm_bw_transport_if<> > Initiator;
};

template <unsigned int BUSWIDTH = 32, int N = 1>
class TlmSocket {
public:
    typedef tlm::tlm_target_socket<BUSWIDTH,
                                   tlm::tlm_base_protocol_types,
                                   N> target;
    typedef tlm::tlm_initiator_socket<BUSWIDTH,
                                   tlm::tlm_base_protocol_types,
                                   N> initiator;
};

class MemoryMappingInspectorScIface : public virtual sc_core::sc_interface {
public:
    virtual const std::vector<AddressRange> & get_memory_mapping() const = 0;
};

template <unsigned int BUSWIDTH = 32>
class TlmBusIface : public MemoryMappingInspectorScIface {
public:
    virtual void connect_target(typename TlmSocketBase<BUSWIDTH>::Target &s, const AddressRange &r) = 0;
    virtual void connect_initiator(typename TlmSocketBase<BUSWIDTH>::Initiator &s) = 0;
    virtual sc_core::sc_module* get_sc_module() = 0;
};

class DmiInfo {
public:
    void * ptr;

    AddressRange range;

    bool read_allowed;
    bool write_allowed;

    sc_core::sc_time read_latency;
    sc_core::sc_time write_latency;

    bool is_read_write_allowed() const { return read_allowed && write_allowed; }
};

struct BusAccessResponseStatus
{
public:
    enum val {
        OK                = tlm::TLM_OK_RESPONSE,
        INCOMPLETE        = tlm::TLM_INCOMPLETE_RESPONSE,
        GENERIC_ERROR     = tlm::TLM_GENERIC_ERROR_RESPONSE,
        ADDRESS_ERROR     = tlm::TLM_ADDRESS_ERROR_RESPONSE,
        COMMAND_ERROR     = tlm::TLM_COMMAND_ERROR_RESPONSE,
        BURST_ERROR       = tlm::TLM_BURST_ERROR_RESPONSE,
        BYTE_ENABLE_ERROR = tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE,
    };

private:
    val m_val;

public:
    BusAccessResponseStatus(const tlm::tlm_response_status &t)
    {
        m_val = static_cast<val>(t);
    }

    BusAccessResponseStatus(const val &v)
    {
        m_val = v;
    }

    BusAccessResponseStatus& operator= (const tlm::tlm_response_status &t)
    {
        m_val = static_cast<val>(t);
        return *this;
    }

    BusAccessResponseStatus& operator= (const val &v)
    {
        m_val = v;
        return *this;
    }

    bool is_error() const { return m_val != OK; }
};

#endif
