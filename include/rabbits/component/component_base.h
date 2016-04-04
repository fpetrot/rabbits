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

/**
 * @brief Slave interface.
 *
 * If a class implements this interface, it is considered being a slave
 * component connectable to a Bus.
 */
class SlaveIface : public tlm::tlm_fw_transport_if<> {
public:

    /**
     * @brief Get the ComponentBase associated to this slave.
     *
     * @return the base component.
     */
    virtual ComponentBase& get_component() = 0;

    /**
     * @brief Set the BusSlaveIfaceBase associated to this slave.
     *
     * @param[in] iface the bus interface.
     */
    virtual void set_bus_iface(BusSlaveIfaceBase *iface) = 0;

    /**
     * @brief Check if this slave as already a bus interface.
     *
     * @return true if this slave already has a bus interface, false otherwise.
     */
    virtual bool bus_iface_is_set() = 0;

    /**
     * @brief Get the slave bus interface.
     *
     * @return the slave bus interface associated to the slave. If the bus
     * interface is not set, the behavior is undefined.
     */
    virtual BusSlaveIfaceBase & get_bus_iface() = 0;
};

/**
 * @brief Slave interface.
 *
 * If a class implements this interface, it is considered being a master
 * component connectable to a Bus.
 */
class MasterIface : public tlm::tlm_bw_transport_if<> {
public:
    /**
     * @brief Get the ComponentBase associated to this master.
     *
     * @return the base component.
     */
    virtual ComponentBase& get_component() = 0;

    /**
     * @brief Helper method called by the PlatformBuilder instance at the end of elaboration.
     *
     * Helper method called by the PlatformBuilder instance at the end of
     * elaboration to inform the master of the platform memory mapping. For
     * each slave connect to the bus, this method is called with address start
     * and mapping size information.
     *
     * This method try to initiate a DMI request to the corresponding slave. If
     * the request succeeds, it calls back the dmi_hint_cb method with all DMI
     * information so that the master can use them if it wants to.
     *
     * This method is supposed to be called by an instance of PlatformBuilder only.
     *
     * @param[in] start Start address of the memory mapping
     * @param[in] size Size of the memory mapping
     *
     * @see dmi_hint_cb
     */
    virtual void dmi_hint(uint64_t start, uint64_t size) = 0;

    /**
     * @brief Set the BusSlaveIfaceBase associated to this master.
     *
     * @param[in] iface the bus interface.
     */
    virtual void set_bus_iface(BusMasterIfaceBase *iface) = 0;

    /**
     * @brief Check if this master as already a bus interface.
     *
     * @return true if this master already has a bus interface, false otherwise.
     */
    virtual bool bus_iface_is_set() = 0;

    /**
     * @brief Get the master bus interface.
     *
     * @return the master bus interface associated to the master. If the bus
     * interface is not set, the behavior is undefined.
     */
    virtual BusMasterIfaceBase & get_bus_iface() = 0;
};

/**
 * @brief bus interface.
 *
 * If a class implements this interface, it is considered being a bus
 * component that can interconnect masters and slaves.
 */
class BusIface {
public:
    /**
     * @brief Connect a slave to the bus, mapping it at the given address range.
     *
     * @param[in] SlaveIface The slave to connect to the bus.
     * @param[in] AddressRange The address range where to map the slave.
     */
    virtual void connect_slave(SlaveIface&, AddressRange) = 0;

    /**
     * @brief Connect a master to the bus.
     *
     * @param[in] MasterIface The master to connect to the bus.
     */
    virtual void connect_master(MasterIface&) = 0;
};

/**
 * @brief Child components interface
 *
 * This interface represents an entity having nested components.
 * These child components are identified with a name an can be iterated on.
 */
class HasChildCompIface {
public:
    /**
     * @brief Check if the child component with the given name exists.
     *
     * @param[in] id The component name.
     *
     * @return true if the component exists, false otherwise.
     */
    virtual bool child_component_exists(std::string id) const = 0;

    /**
     * @brief Get the child component with the given name.
     *
     * @param[in] id the component name.
     *
     * @return the component associated with this id.
     * @throw ComponentNotFoundException if the child component does not exist.
     */
    virtual ComponentBase& get_child_component(std::string id) = 0;

    /**
     * @brief Check if the child slave with the given name exists.
     *
     * @param[in] id The slave name.
     *
     * @return true if the slave exists, false otherwise.
     */
    virtual bool child_slave_exists(std::string id) const = 0;

    /**
     * @brief Get the child slave with the given name.
     *
     * @param[in] id the slave name.
     *
     * @return the slave associated with this id.
     * @throw ComponentNotFoundException if the child slave does not exist.
     */
    virtual SlaveIface& get_child_slave(std::string id) = 0;

    /**
     * @brief Check if the child master with the given name exists.
     *
     * @param[in] id The master name.
     *
     * @return true if the master exists, false otherwise.
     */
    virtual bool child_master_exists(std::string id) const = 0;

    /**
     * @brief Get the child master with the given name.
     *
     * @param[in] id the master name.
     *
     * @return the master associated with this id.
     * @throw ComponentNotFoundException if the child master does not exist.
     */
    virtual MasterIface& get_child_master(std::string id) = 0;

    typedef std::map<std::string, ComponentBase*>::iterator component_iterator;
    typedef std::map<std::string, ComponentBase*>::const_iterator const_component_iterator;
    typedef std::map<std::string, MasterIface*>::iterator master_iterator;
    typedef std::map<std::string, MasterIface*>::const_iterator const_master_iterator;
    typedef std::map<std::string, SlaveIface*>::iterator slave_iterator;
    typedef std::map<std::string, SlaveIface*>::const_iterator const_slave_iterator;

    /**
     * @brief Return an iterator to the first child component.
     *
     * @return an iterator to the first child component.
     */
    virtual component_iterator child_component_begin() = 0;

    /**
     * @brief Return an iterator to the <i>past-tho-end</i> child component.
     *
     * @return an iterator to the <i>past-tho-end</i> child component.
     */
    virtual component_iterator child_component_end() = 0;

    /**
     * @brief Return an constant iterator to the first child component.
     *
     * @return an constant iterator to the first child component.
     */
    virtual const_component_iterator child_component_begin() const = 0;

    /**
     * @brief Return an constant iterator to the <i>past-tho-end</i> child component.
     *
     * @return an constant iterator to the <i>past-tho-end</i> child component.
     */
    virtual const_component_iterator child_component_end() const = 0;


    /**
     * @brief Return an iterator to the first child master component.
     *
     * @return an iterator to the first child master component.
     */
    virtual master_iterator child_master_begin() = 0;

    /**
     * @brief Return an iterator to the <i>past-tho-end</i> child master component.
     *
     * @return an iterator to the <i>past-tho-end</i> child master component.
     */
    virtual master_iterator child_master_end() = 0;

    /**
     * @brief Return an constant iterator to the first child master component.
     *
     * @return an constant iterator to the first child master component.
     */
    virtual const_master_iterator child_master_begin() const = 0;

    /**
     * @brief Return an constant iterator to the <i>past-tho-end</i> child master component.
     *
     * @return an constant iterator to the <i>past-tho-end</i> child master component.
     */
    virtual const_master_iterator child_master_end() const = 0;


    /**
     * @brief Return an iterator to the first child slave component.
     *
     * @return an iterator to the first child slave component.
     */
    virtual slave_iterator child_slave_begin() = 0;

    /**
     * @brief Return an iterator to the <i>past-tho-end</i> child slave component.
     *
     * @return an iterator to the <i>past-tho-end</i> child slave component.
     */
    virtual slave_iterator child_slave_end() = 0;

    /**
     * @brief Return an constant iterator to the first child slave component.
     *
     * @return an constant iterator to the first child slave component.
     */
    virtual const_slave_iterator child_slave_begin() const = 0;

    /**
     * @brief Return an constant iterator to the <i>past-tho-end</i> child slave component.
     *
     * @return an constant iterator to the <i>past-tho-end</i> child slave component.
     */
    virtual const_slave_iterator child_slave_end() const = 0;
};

/**
 * @brief Component base class.
 */
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

    /**
     * @brief Return the component parameters.
     *
     * @return the component parameters.
     */
    const ComponentParameters & get_params() { return m_params; }

    /* HasLoggerIface */
    std::ostream & log_stream(LogLevel::value lvl) const
    {
        LogLevel::value eff = lvl, save;

        if (m_loglvl_override) {
            save = Logger::get().get_log_level();
            eff = m_loglvl;
            Logger::get().set_log_level(eff);
        }

        std::ostream & ret = ::log_stream(lvl);

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
        int ret = ::log_vprintf(lvl, fmt, ap);
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
