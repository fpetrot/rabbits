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

#include <sstream>
#include <set>

#include "rabbits/platform/builder.h"

#include "rabbits/logger.h"
#include "rabbits/platform/description.h"
#include "rabbits/component/factory.h"
#include "rabbits/component/component.h"
#include "rabbits/datatypes/address_range.h"

#include "rabbits/plugin/hook.h"
#include "rabbits/plugin/manager.h"

#include "rabbits/component/slave.h"
#include "rabbits/component/master.h"
#include "rabbits/component/bus.h"

using namespace sc_core;
using std::string;
using std::stringstream;


PlatformBuilder::PlatformBuilder(sc_module_name name, PlatformDescription &descr)
    : sc_module(name), m_dbg("dbg-initiator"), m_bus(NULL)
{
    PluginManager &pm = PluginManager::get();

    pm.run_hook(PluginHookBeforeBuild(&descr, this));

    create_components(descr, CreationStage::DISCOVER);
    pm.run_hook(PluginHookAfterComponentDiscovery(&descr, this));
    
    create_components(descr, CreationStage::CREATE);
    pm.run_hook(PluginHookAfterComponentInst(&descr, this));
    
    do_bus_connections(descr);
    pm.run_hook(PluginHookAfterBusConnections(&descr, this));

    do_irq_connections(descr);

    if (m_bus) {
        m_bus->connect_master(m_dbg);
    }

    pm.run_hook(PluginHookAfterBuild(&descr, this));
}

PlatformBuilder::~PlatformBuilder()
{
    std::vector< sc_signal<bool>* >::iterator it;

    for (it = m_sigs.begin(); it != m_sigs.end(); it++) {
        delete *it;
    }
}

void PlatformBuilder::create_components(PlatformDescription &descr, CreationStage::value stage)
{
    PlatformDescription::iterator it;
    ComponentManager &cm = ComponentManager::get();

    if ((!descr.exists("components")) || (descr["components"].type() != PlatformDescription::MAP)) {
        WRN_STREAM("No component found in description\n");
        return;
    }

    for (it = descr["components"].begin(); it != descr["components"].end(); it++) {
        ComponentFactory *cf = NULL;
        const string &name = it->first;
        PlatformDescription &comp = it->second;

        if (!comp.exists("type")) {
            WRN_STREAM("Missing `type` attribute for component `" << name << "`\n");
            continue;
        }

        const string type = comp["type"].as<string>();

        cf = cm.find_by_type(type);

        if (cf != NULL) {
            switch (stage) {
            case CreationStage::DISCOVER:
                cf->discover(name, comp);
                break;
            case CreationStage::CREATE:
                DBG_STREAM("Creating component " << name << " of type " << type << "\n");
                m_components[name] = cf->create(name, comp);
                break;
            }
        } else {
            WRN_STREAM("No component type can provide `" << type << "`\n");
        }
    }
}

void PlatformBuilder::connect_master(MasterIface *m, Bus *b)
{
    if (m_connected.find(&(m->get_component())) == m_connected.end()) {
        DBG_STREAM("Bus mapping: " 
                   << m->get_component().name() 
                   << " -> " << b->name() << "\n");

        b->connect_master(*m);
        m_masters.push_back(m);
        m_connected.insert(&(m->get_component()));
    }
}

void PlatformBuilder::connect_slave(SlaveIface *s, Bus *b, const AddressRange& r)
{
    if (m_connected.find(&(s->get_component())) == m_connected.end()) {
        DBG_STREAM("Bus mapping: " 
                   << s->get_component().name() 
                   << " -> " << b->name() << r << "\n");
        b->connect_slave(*s, r);
        m_mappings.push_back(r);
        m_connected.insert(&(s->get_component()));
    }
}

Bus* PlatformBuilder::get_bus_parent(PlatformDescription &descr)
{
    ComponentBase *c_bus = NULL;
    Bus *bus = NULL;

    if ((!descr.exists("bus-parent")) || (descr["bus-parent"].type() != PlatformDescription::SCALAR)) {
        return NULL;
    }
    
    string n = descr["bus-parent"].as<string>();

    if (m_components.find(n) == m_components.end()) {
        WRN_STREAM("Bus component `" << n << "` not found\n";);
        return NULL;
    }

    c_bus = m_components[n];

    bus = dynamic_cast<Bus*>(c_bus);

    if (bus == NULL) {
        WRN_STREAM("Component `" << n << "` is not a bus\n");
        return NULL;
    }

    return bus;
}

bool PlatformBuilder::get_bus_mapping(PlatformDescription &descr, AddressRange &ret)
{
    if ((!descr.exists("bus-mapping")) || (descr["bus-mapping"].type() != PlatformDescription::MAP)) {
        return false;
    }

    ret = descr["bus-mapping"].as<AddressRange>();
    return true;
}

void PlatformBuilder::do_master_child_bus_connections(ComponentBase *c, Bus *b)
{
    HasChildCompIface::master_iterator it;

    for (it = c->child_master_begin(); it != c->child_master_end(); it++) {
        connect_master(it->second, b);
    }
}
void PlatformBuilder::do_child_bus_connections(ComponentBase *c, Bus *b, PlatformDescription &descr)
{
    PlatformDescription::iterator it;

    do_master_child_bus_connections(c, b);

    if (!descr.exists("child-bus-mapping")) {
        return;
    }

    if (!descr.is_map()) {
        WRN_STREAM("Invalid child-bus-mapping attribute for component `"
                   << c->name() << "`\n");
        return;
    }


    for (it = descr["child-bus-mapping"].begin(); it != descr["child-bus-mapping"].end(); it++) {
        const string &name = it->first;
        PlatformDescription &d = it->second;

        if (!c->child_slave_exists(name)) {
            WRN_STREAM("Slave child `" << name << "` not found for component `"
                       << c->name() << "`\n");
            continue;
        }

        if (!d.is_map()) {
            WRN_STREAM("Invalid mapping description for slave child `"
                       << name << "` of component `" << c->name() << "`\n");
            continue;
        }

        SlaveIface &s = c->get_child_slave(name);
        AddressRange mapping = d.as<AddressRange>();

        connect_slave(&s, b, mapping);
    }
}

void PlatformBuilder::do_bus_connections(PlatformDescription &descr)
{
    PlatformDescription::iterator it;
    SlaveIface *s = NULL;
    MasterIface *m = NULL;
    BusIface *bus = NULL;

    if ((!descr.exists("components")) || (descr["components"].type() != PlatformDescription::MAP)) {
        return;
    }

    for (it = descr["components"].begin(); it != descr["components"].end(); it++) {
        const string &name = it->first;
        PlatformDescription &comp_descr = it->second;

        if (m_components.find(name) == m_components.end()) {
            continue;
        }

        ComponentBase *comp = m_components[name];
        Bus *bus_parent = get_bus_parent(comp_descr);

        if (bus_parent) {
            if ((s = dynamic_cast<SlaveIface*>(comp)) != NULL) {
                AddressRange mapping;

                if(!get_bus_mapping(comp_descr, mapping)) {
                    WRN_STREAM("Missing or invalid attribute `bus-mapping` for component `"
                               << name << "`\n");
                    continue;
                }

                connect_slave(s, bus_parent, mapping);

            } else if((m = dynamic_cast<MasterIface*>(comp)) != NULL) {
                connect_master(m, bus_parent);
            }
        } else if ((bus = dynamic_cast<BusIface*>(comp)) != NULL) {
            /* Top-level bus */
            m_bus = bus;

        } else {
            WRN_STREAM("Component `" << name << "` has no or invalid `bus-parent` attribute\n");
            continue;
        }

        do_child_bus_connections(comp, bus_parent, comp_descr);
    }
}



ComponentBase* PlatformBuilder::get_irq_parent(PlatformDescription &descr)
{
    if ((!descr.exists("irq-parent")) || (descr["irq-parent"].type() != PlatformDescription::SCALAR)) {
        return NULL;
    }
    
    string n = descr["irq-parent"].as<string>();

    if (m_components.find(n) == m_components.end()) {
        WRN_STREAM("IRQ parent component `" << n << "` not found\n";);
        return NULL;
    }

    return m_components[n];
}

IrqIn* PlatformBuilder::get_irq_in(ComponentBase &c, const string &id)
{
    unsigned int idx;
    if (c.irq_in_exists(id)) {
        /* As id */
        return &(c.get_irq_in(id));
    } else if ((stringstream(id) >> idx) && (c.irq_in_exists(idx))) {
        /* As idx */
        return &(c.get_irq_in(idx));
    } else {
        return NULL;
    }
}

IrqOut* PlatformBuilder::get_irq_out(ComponentBase &c, const string &id)
{
    unsigned int idx;
    if (c.irq_out_exists(id)) {
        /* As id */
        return &(c.get_irq_out(id));
    } else if ((stringstream(id) >> idx) && (c.irq_out_exists(idx))) {
        /* As idx */
        return &(c.get_irq_out(idx));
    } else {
        return NULL;
    }
}

ComponentBase* PlatformBuilder::get_irq_parent_from_mapping(const string &mapping, string &final_irq)
{
    size_t dot = mapping.find_first_of('.');

    if (dot == string::npos) {
        return NULL;
    }

    string cname = mapping.substr(0, dot);

    if (m_components.find(cname) == m_components.end()) {
        WRN_STREAM("IRQ parent component `" << cname << "` not found\n";);
        return NULL;
    }

    final_irq = mapping.substr(dot+1, string::npos);
    return m_components[cname];
}

void PlatformBuilder::get_irq_mapping(PlatformDescription &descr,
                                      ComponentBase &src, ComponentBase *dst, 
                                      std::vector< IrqPair > &pairs)
{
    PlatformDescription::iterator it;
    PlatformDescription &map = descr["irq-mapping"];
    IrqOut *irq_out;
    IrqIn *irq_in;
    std::set<string> mapped_out;
    HasIrqOutIface::iterator irq_it;

    switch (map.type()) {
    case PlatformDescription::MAP:
        for (it = map.begin(); it != map.end(); it++) {
            const string &src_irq = it->first;
            irq_out = get_irq_out(src, src_irq);

            if (irq_out == NULL) {
                WRN_STREAM("No irq out named `" << src_irq
                           << "` for component `" << src.name() << "`\n");
                continue;
            }

            if (it->second.type() != PlatformDescription::SCALAR) {
                WRN_STREAM("Invalid IRQ mapping. Ignoring\n");
                continue;
            }

            const string &dst_irq = it->second.as<string>();

            ComponentBase *d;
            string final_irq;

            if (dst == NULL) {
                d = get_irq_parent_from_mapping(dst_irq, final_irq);
                if (d == NULL) {
                    continue;
                }
            } else {
                d = dst;
                final_irq = dst_irq;
            }
            
            irq_in = get_irq_in(*d, final_irq);

            if (irq_in == NULL) {
                WRN_STREAM("No irq in named `" << final_irq
                           << "` for component `" << d->name() << "`\n");
                continue;
            }

            pairs.push_back(IrqPair(irq_in, irq_out));
            mapped_out.insert(irq_out->name());
        }
        break;

    case PlatformDescription::SCALAR:
        /* Take the first src irq out and connect it to dst, to the irq designed by the scalar */
        irq_out = get_irq_out(src, "0");

        if (irq_out == NULL) {
            WRN_STREAM("No irq out found for component `"
                       << src.name() << "`\n");
            break;
        }

        {
            const string &dst_irq = map.as<string>();
            irq_in = get_irq_in(*dst, dst_irq);

            if (irq_in == NULL) {
                WRN_STREAM("No irq in named `" << dst_irq
                           << "` for component `" << dst->name() << "`\n");
                break;
            }
        }

        pairs.push_back(IrqPair(irq_in, irq_out));
        mapped_out.insert(irq_out->name());
        break;

    case PlatformDescription::VECTOR:
    case PlatformDescription::NIL:
    case PlatformDescription::INVALID:
        break;
    }

    if (dst != NULL) {
        /* Automatic connections of irqs with same name */
        for (irq_it = src.irqs_out_begin(); irq_it != src.irqs_out_end(); irq_it++) {
            if (mapped_out.find(irq_it->first) != mapped_out.end()) {
                /* Already manually connected */
                continue;
            }

            if (dst->irq_in_exists(irq_it->first)) {
                irq_out = irq_it->second;
                irq_in = &(dst->get_irq_in(irq_it->first));

                pairs.push_back(IrqPair(irq_in, irq_out));
            }
        }
    }

    for (std::vector<IrqPair>::iterator it = pairs.begin(); it != pairs.end(); it++) {
        irq_in = it->first;
        irq_out = it->second;
        DBG_STREAM("IRQ mapping: " 
                   << src.name() << ":" << irq_out->name() << " -> " 
                   << (dst ? dst->name(): "?") << ":" << irq_in->name() << "\n");
    }
}

void PlatformBuilder::do_irq_connections(PlatformDescription &descr)
{
    PlatformDescription::iterator it;

    if ((!descr.exists("components")) || (descr["components"].type() != PlatformDescription::MAP)) {
        return;
    }

    for (it = descr["components"].begin(); it != descr["components"].end(); it++) {
        const string &name = it->first;
        PlatformDescription &comp_descr = it->second;

        if (m_components.find(name) == m_components.end()) {
            continue;
        }

        ComponentBase *comp = m_components[name];
        ComponentBase *irq_parent = get_irq_parent(comp_descr);

	std::vector< IrqPair > pairs;
	std::vector< IrqPair >::iterator it;

	get_irq_mapping(comp_descr, *comp, irq_parent, pairs);

	for (it = pairs.begin(); it != pairs.end(); it++) {
		sc_signal<bool> *s = NULL;

		s = it->first->connect(*(it->second));

		if (s) {
			m_sigs.push_back(s);
		}
	}
    }
}

void PlatformBuilder::end_of_elaboration()
{
    std::vector<MasterIface *>::iterator master;
    std::vector<AddressRange>::iterator descr;

    /* Inform masters of mapped addresses */
    for (master = m_masters.begin(); master != m_masters.end(); master++) {
        for (descr = m_mappings.begin(); descr != m_mappings.end(); descr++) {
            (*master)->dmi_hint(descr->begin(), descr->size());
        }
    }
}

