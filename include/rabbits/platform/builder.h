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

#ifndef _UTILS_PLATFORM_BUILDER_H
#define _UTILS_PLATFORM_BUILDER_H

#include <systemc>

#include <vector>
#include <map>
#include <set>

#include "rabbits/component/manager.h"
#include "rabbits/component/component.h"
#include "rabbits/component/debug_initiator.h"
#include "rabbits/component/bus.h"

class PlatformDescription;
class AddressRange;
class Master;

class PlatformBuilder : public sc_core::sc_module {
public:
    typedef std::map<std::string, ComponentBase*>::iterator comp_iterator;
    typedef std::map<std::string, ComponentBase*>::const_iterator const_comp_iterator;
protected:
    struct CreationStage {
        enum value { DISCOVER, CREATE };
    };

    typedef std::pair<IrqIn*, IrqOut*> IrqPair;

    std::map<std::string, ComponentBase*> m_components;
    std::vector<MasterIface*> m_masters;
    std::vector<AddressRange> m_mappings;
    std::set<ComponentBase*> m_connected;

    DebugInitiator m_dbg;
    BusIface *m_bus;

    std::vector< sc_core::sc_signal<bool>* > m_sigs;

    void create_components(PlatformDescription &descr, CreationStage::value);

    void connect_master(MasterIface *m, Bus *b);
    void connect_slave(SlaveIface *s, Bus *b, const AddressRange&);

    void do_child_bus_connections(ComponentBase *c, Bus *b, PlatformDescription &descr);
    void do_master_child_bus_connections(ComponentBase *c, Bus *b);
    void do_bus_connections(PlatformDescription &descr);
    void do_irq_connections(PlatformDescription &descr);

    Bus * get_bus_parent(PlatformDescription &descr);
    bool get_bus_mapping(PlatformDescription &descr, AddressRange&);

    ComponentBase * get_irq_parent(PlatformDescription &descr);
    ComponentBase * get_irq_parent_from_mapping(const std::string &mapping, std::string &final_irq);
    void get_irq_mapping(PlatformDescription &descr,
                         ComponentBase &src, ComponentBase *dst,
                         std::vector< IrqPair > &pairs);

    IrqIn* get_irq_in(ComponentBase &c, const std::string &id);
    IrqOut* get_irq_out(ComponentBase &c, const std::string &id);

    virtual void end_of_elaboration();

public:
    SC_HAS_PROCESS(PlatformBuilder);
    PlatformBuilder(sc_core::sc_module_name name, PlatformDescription &descr);
    virtual ~PlatformBuilder();

    DebugInitiator& get_dbg_init() { return m_dbg; }

    comp_iterator comp_begin() { return m_components.begin(); }
    comp_iterator comp_end() { return m_components.end(); }
    const_comp_iterator comp_begin() const { return m_components.begin(); }
    const_comp_iterator comp_end() const { return m_components.end(); }

    bool comp_exists(const std::string &name) const { return m_components.find(name) != m_components.end(); }
    ComponentBase & get_comp(const std::string &name) {
        if (!comp_exists(name)) {
            throw ComponentNotFoundException(name);
        }

        return *m_components[name];
    }
};

#endif
