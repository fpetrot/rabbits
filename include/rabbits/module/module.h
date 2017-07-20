/*
 *  This file is part of Rabbits
 *  Copyright (C) 2015-2017  Clement Deschamps and Luc Michel
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

#ifndef _RABBITS_MODULE_MODULE_H
#define _RABBITS_MODULE_MODULE_H

#include "rabbits/logger/has_logger.h"
#include "rabbits/config/has_config.h"
#include "has_params.h"
#include "namespace.h"

class ModuleFactoryBase;

class ModuleIface
    : public HasParametersIface
    , public HasLoggerIface
    , public HasConfigIface
{
public:
    /**
     * @brief Return the module name
     */
    virtual const std::string & get_name() const = 0;

    /**
     * @brief Return the module namespace
     */
    virtual const Namespace & get_namespace() const = 0;

    /* @brief Return the full name of the module
     *
     * The full name is composed of the name of the namespace, a ".", and the
     * name of the module.
     */
    virtual const std::string get_full_name() const = 0;

    /**
     * @brief Return the factory that were used to build this module
     */
    virtual ModuleFactoryBase * get_factory() const = 0;
};

#endif
