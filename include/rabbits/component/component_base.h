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

#ifndef _RABBITS_COMPONENT_COMPONENT_BASE_H
#define _RABBITS_COMPONENT_COMPONENT_BASE_H

#include <map>

#include <systemc>
#include <tlm>

#include "rabbits/module/parameters.h"
#include "port.h"
#include "rabbits/rabbits_exception.h"
#include "rabbits/datatypes/address_range.h"
#include "rabbits/platform/description.h"
#include "rabbits/logger.h"

/**
 * @brief Exception raised when a named component has not been found.
 */
class ComponentNotFoundException : public RabbitsException {
protected:
    std::string make_what(std::string comp) { return "Component `" + comp + "` not found."; }
public:
    explicit ComponentNotFoundException(const std::string & comp) : RabbitsException(make_what(comp)) {}
    virtual ~ComponentNotFoundException() throw() {}
};

class HasAttributesIface
{
public:
    virtual void add_attr(const std::string & key, const std::string & value) = 0;
    virtual bool has_attr(const std::string & key) = 0;
    virtual std::string get_attr(const std::string & key) = 0;
};

/**
 * @brief Component base class.
 */
class ComponentBase
    : public sc_core::sc_module
    , public HasPortIface
    , public HasAttributesIface
    , public HasLoggerIface {
public:
    typedef std::map<std::string, std::string> Attributes;

protected:
    Parameters m_params;

public:
    ComponentBase(sc_core::sc_module_name name, const Parameters &params)
        : sc_core::sc_module(name)
        , m_params(params)
    {}

    ComponentBase(sc_core::sc_module_name name)
        : sc_core::sc_module(name)
    {}

    virtual ~ComponentBase() { }

    /**
     * @brief Return the component parameters.
     *
     * @return the component parameters.
     */
    const Parameters & get_params() { return m_params; }
};
#endif
