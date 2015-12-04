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

#ifndef _DEBUG_INITIATOR_H
#define _DEBUG_INITIATOR_H

#include "rabbits/component/component.h"
#include "rabbits/component/master.h"

#include <tlm_utils/simple_initiator_socket.h>


class DebugInitiator : public Master {
public:
    DebugInitiator(std::string name);
    DebugInitiator(std::string name, ComponentParameters &cp);
    virtual ~DebugInitiator();

    uint64_t debug_read(uint64_t addr, void *buf, uint64_t size);
    uint64_t debug_write(uint64_t addr, void *buf, uint64_t size);
};

#endif
