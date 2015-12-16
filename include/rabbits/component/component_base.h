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

#include <systemc>
#include <tlm>

#include "irq.h"
#include "parameters.h"
#include "rabbits/rabbits_exception.h"
#include "rabbits/datatypes/address_range.h"

class ComponentBase;
class SlaveIface;
class MasterIface;
class BusMasterIfaceBase;

class ComponentNotFoundException : public RabbitsException {
protected:
    std::string make_what(std::string comp) { return "Component `" + comp + "` not found."; }
public:
    explicit ComponentNotFoundException(const std::string & comp) : RabbitsException(make_what(comp)) {}
    virtual ~ComponentNotFoundException() throw() {}
};

class SlaveIface : public tlm::tlm_fw_transport_if<> {
public:
    virtual ComponentBase& get_component() = 0;
};

class MasterIface : public tlm::tlm_bw_transport_if<> {
public:
    virtual ComponentBase& get_component() = 0;

    virtual void set_bus_iface(BusMasterIfaceBase *iface) = 0;
    virtual void dmi_hint(uint64_t start, uint64_t size) = 0;
};

class BusIface {
public:
    virtual void connect_slave(SlaveIface&, AddressRange) = 0;
    virtual void connect_master(MasterIface&) = 0;
};

class HasChildCompIface {
public:
    virtual bool child_component_exists(std::string id) const = 0;
    virtual ComponentBase& get_child_component(std::string id) = 0;

    virtual bool child_slave_exists(std::string id) const = 0;
    virtual SlaveIface& get_child_slave(std::string id) = 0;

    virtual bool child_master_exists(std::string id) const = 0;
    virtual MasterIface& get_child_master(std::string id) = 0;

    typedef std::map<std::string, ComponentBase*>::iterator component_iterator;
    typedef std::map<std::string, ComponentBase*>::const_iterator const_component_iterator;
    typedef std::map<std::string, MasterIface*>::iterator master_iterator;
    typedef std::map<std::string, MasterIface*>::const_iterator const_master_iterator;
    typedef std::map<std::string, SlaveIface*>::iterator slave_iterator;
    typedef std::map<std::string, SlaveIface*>::const_iterator const_slave_iterator;

    virtual component_iterator child_component_begin() = 0;
    virtual component_iterator child_component_end() = 0;
    virtual const_component_iterator child_component_begin() const = 0;
    virtual const_component_iterator child_component_end() const = 0;

    virtual master_iterator child_master_begin() = 0;
    virtual master_iterator child_master_end() = 0;
    virtual const_master_iterator child_master_begin() const = 0;
    virtual const_master_iterator child_master_end() const = 0;

    virtual slave_iterator child_slave_begin() = 0;
    virtual slave_iterator child_slave_end() = 0;
    virtual const_slave_iterator child_slave_begin() const = 0;
    virtual const_slave_iterator child_slave_end() const = 0;
};

class ComponentBase 
    : public sc_core::sc_module
    , public HasIrqInIface
    , public HasIrqOutIface
    , public HasChildCompIface {
protected:
    ComponentParameters m_params;
    
public:
    ComponentBase(std::string name, const ComponentParameters &params)
        : sc_core::sc_module(sc_core::sc_module_name(name.c_str())), m_params(params) {}
    ComponentBase(std::string name) : sc_core::sc_module(sc_core::sc_module_name(name.c_str())) {}
    virtual ~ComponentBase() {}

    const ComponentParameters & get_params() { return m_params; }
};
#endif
