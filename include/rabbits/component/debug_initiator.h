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
 * @file debug_initiator.h
 * @brief DebugInitiator class declaration
 */

#ifndef _DEBUG_INITIATOR_H
#define _DEBUG_INITIATOR_H

#include "rabbits/component/component.h"
#include "rabbits/component/master.h"

#include <tlm_utils/simple_initiator_socket.h>


/**
 * @brief Helper class to emit debug requests on a bus.
 *
 * This helper component allows to easily emits read or write debug requests on
 * the bus it is connected to.
 */
class DebugInitiator : public Master<> {
public:
    DebugInitiator(sc_core::sc_module_name name, ConfigManager &config);
    DebugInitiator(sc_core::sc_module_name name, Parameters &cp, ConfigManager &config);
    virtual ~DebugInitiator();

    /**
     * @brief Emit a read debug request on the bus.
     *
     * @param[in] addr Address of the read request.
     * @param[out] buf Array where to write the result of the request.
     * @param[in] size Size of the read request.
     *
     * @return the number of bytes effectively read.
     */
    uint64_t debug_read(uint64_t addr, void *buf, uint64_t size);

    /**
     * @brief Emit a write debug request on the bus.
     *
     * @param[in] addr Address of the write request.
     * @param[in] buf Array containing the data to write.
     * @param[in] size Size of the write request.
     *
     * @return the number of bytes effectively written.
     */
    uint64_t debug_write(uint64_t addr, const void *buf, uint64_t size);
};

#endif
