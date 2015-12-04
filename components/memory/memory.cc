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

#include "memory.h"

#include <cstdio>
#include <cstdlib>

#include <rabbits/logger.h>

using namespace sc_core;

Memory::Memory(const char *name, uint64_t size)
    : Slave(name)
    , MEM_WRITE_LATENCY(3, SC_NS)
    , MEM_READ_LATENCY(3, SC_NS)
{
    m_size = size;
    m_bytes = new unsigned char[m_size];
    DBG_PRINTF("memory: Memory area location: %p\n", m_bytes);
}

Memory::Memory(const std::string &name, ComponentParameters &params)
    : Slave(name, params)
    , MEM_WRITE_LATENCY(3, SC_NS)
    , MEM_READ_LATENCY(3, SC_NS)
{
    m_size = params["size"].as<uint64_t>();
    m_bytes = new unsigned char[m_size];
#ifdef PARANOID_INIT
    memset (m_bytes, 0, m_size);
#endif
    DBG_PRINTF("memory: Memory area location: %p\n", m_bytes);
}


Memory::~Memory()
{
    if (m_bytes)
        delete[] m_bytes;
}

void Memory::bus_cb_read(uint64_t addr, uint8_t *data, unsigned int len, bool &bErr)
{
    if(addr + len >= m_size) {
        printf("memory: error : reading outside bounds\n");
        exit(1);
    }

    memcpy(data, m_bytes + addr, len);

    wait(MEM_READ_LATENCY);
}

void Memory::bus_cb_write(uint64_t addr, uint8_t *data, unsigned int len, bool &bErr)
{
    if(addr + len >= m_size) {
        printf("memory: error : writing outside bounds\n");
        exit(1);
    }

    wait(MEM_WRITE_LATENCY);

    memcpy(m_bytes + addr, data, len);
}

uint64_t Memory::debug_read(uint64_t addr, uint8_t *buf, uint64_t size)
{
    unsigned int to_read = (addr + size > m_size) ? m_size - addr : size;

    memcpy(buf, m_bytes + addr, to_read);

    return to_read;
}

uint64_t Memory::debug_write(uint64_t addr, const uint8_t *buf, uint64_t size)
{
    unsigned int to_write = (addr + size > m_size) ? m_size - addr : size;

    memcpy(m_bytes + addr, buf, to_write);

    return to_write;
}

bool Memory::get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                                tlm::tlm_dmi& dmi_data)
{
    if (trans.get_address() > m_size) {
        return false;
    }

    dmi_data.set_start_address(0);
    dmi_data.set_end_address(m_size-1);
    dmi_data.set_dmi_ptr(m_bytes);
    dmi_data.set_granted_access(tlm::tlm_dmi::DMI_ACCESS_READ_WRITE);
    dmi_data.set_write_latency(MEM_WRITE_LATENCY);
    dmi_data.set_read_latency(MEM_READ_LATENCY);

    return true;
}
