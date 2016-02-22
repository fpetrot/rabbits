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
#include "rabbits/logger/has_logger_iface.h"
#include "rabbits/logger.h"

class ComponentBase;
class SlaveIface;
class MasterIface;
class BusSlaveIfaceBase;
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

    virtual void set_bus_iface(BusSlaveIfaceBase *iface) = 0;
    virtual bool bus_iface_is_set() = 0;
    virtual BusSlaveIfaceBase & get_bus_iface() = 0;
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
    , public HasChildCompIface
    , public HasLoggerIface {
private:
    bool m_loglvl_override;
    LogLevel::value m_loglvl;

protected:
    ComponentParameters m_params;
    
public:
    ComponentBase(sc_core::sc_module_name name, const ComponentParameters &params)
        : sc_core::sc_module(name)
        , m_loglvl_override(false)
        , m_params(params) 
    {
        PlatformDescription &d = m_params.get_base_description();

        if (d["debug"].is_scalar()) {
            bool ov = d["debug"].as<bool>();

            m_loglvl_override = ov;
            m_loglvl = LogLevel::DEBUG;
        }
    }

    ComponentBase(sc_core::sc_module_name name) 
        : sc_core::sc_module(name)
        , m_loglvl_override(false)
    {}

    virtual ~ComponentBase() {}

    const ComponentParameters & get_params() { return m_params; }

    std::ostream & log_stream(LogLevel::value lvl) const
    {
        LogLevel::value eff = lvl, save;

        if (m_loglvl_override) {
            save = Logger::get().get_log_level();
            eff = m_loglvl;
            Logger::get().set_log_level(eff);
        }

        std::ostream & ret = ::log_stream(eff);

        if (m_loglvl_override) {
            Logger::get().set_log_level(save);
        }

        return ret << "[" << name() << "] ";
    }

    int log_vprintf(LogLevel::value lvl, const std::string fmt, va_list ap) const
    {
        LogLevel::value eff = lvl, save;

        if (m_loglvl_override) {
            save = Logger::get().get_log_level();
            eff = m_loglvl;
            Logger::get().set_log_level(eff);
        }

        ::log_printf(eff, "[%s] ", name());
        bool banner = Logger::get().enable_banner(false);
        int ret = ::log_vprintf(eff, fmt, ap);
        Logger::get().enable_banner(banner);

        if (m_loglvl_override) {
            Logger::get().set_log_level(save);
        }

        return ret;
    }

    int log_printf(LogLevel::value lvl, const std::string fmt, ...) const
    {
        va_list ap;
        int written;

        va_start(ap, fmt);
        written = log_vprintf(lvl, fmt, ap);
        va_end(ap);

        return written;
    }
};
#endif
