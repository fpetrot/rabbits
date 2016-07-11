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

#include "rabbits-common.h"
#include "rabbits/component/debug_initiator.h"

DebugInitiator::DebugInitiator(sc_core::sc_module_name n) : Master(n)
{
}

DebugInitiator::DebugInitiator(sc_core::sc_module_name n, Parameters &cp) : Master(n, cp)
{
}

DebugInitiator::~DebugInitiator()
{
}


uint64_t DebugInitiator::debug_read(uint64_t addr, void *buf, uint64_t size)
{
    return static_cast<uint64_t>(
        p_bus.debug_read(addr, reinterpret_cast<uint8_t*>(buf), size));
}

uint64_t DebugInitiator::debug_write(uint64_t addr, void *buf, uint64_t size)
{
    return static_cast<uint64_t>(
        p_bus.debug_write(addr, reinterpret_cast<uint8_t*>(buf), size));
}
