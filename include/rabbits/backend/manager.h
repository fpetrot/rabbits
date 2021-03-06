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

#ifndef _RABBITS_BACKEND_MANAGER_H
#define _RABBITS_BACKEND_MANAGER_H

#include "rabbits/module/manager.h"
#include "factory.h"

/**
 * @brief Backend manager
 *
 * Handles the backends collection. Backends are special components that
 * interacts with te outside world during simulation (user input/output, real
 * peripheral, ...).  This manager allows for searching a backend by name and
 * returns the corresponding backend factory.
 */
class BackendManager : public ModuleManager<BackendFactoryBase> {
public:
    BackendManager() {}
    virtual ~BackendManager() {}
};

#endif
