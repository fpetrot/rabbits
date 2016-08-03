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
#include <vector>

#include "rabbits/platform/builder.h"

#include "rabbits/logger.h"
#include "rabbits/platform/description.h"
#include "rabbits/component/factory.h"
#include "rabbits/datatypes/address_range.h"

#include "rabbits/plugin/plugin.h"
#include "rabbits/plugin/factory.h"
#include "rabbits/plugin/hook.h"
#include "rabbits/plugin/manager.h"

using namespace sc_core;
using std::string;
using std::stringstream;
using std::vector;

static inline void report_parse_warning(const string &msg, const PlatformDescription &descr)
{
    LOG(APP, WRN) << msg << " (at " << descr.origin() << ")\n";
}

PlatformBuilder::PlatformBuilder(sc_module_name name, PlatformDescription &descr,
                                 ConfigManager &config)
    : sc_module(name), m_config(config), m_parser(string(name), descr, config)
{
    create_plugins(m_parser);
    run_hooks(PluginHookBeforeBuild(descr, *this, m_parser));

    create_components(m_parser, CreationStage::DISCOVER);
    run_hooks(PluginHookAfterComponentDiscovery(descr, *this, m_parser));

    create_components(m_parser, CreationStage::CREATE);
    run_hooks(PluginHookAfterComponentInst(descr, *this, m_parser));

    create_backends(m_parser);
    run_hooks(PluginHookAfterBackendInst(descr, *this, m_parser));

    m_parser.instanciation_done();

    do_bindings(m_parser);
    run_hooks(PluginHookAfterBindings(descr, *this, m_parser));

    create_dbg_init(m_parser);

    run_hooks(PluginHookAfterBuild(descr, *this, m_parser));
}

PlatformBuilder::~PlatformBuilder()
{
    delete m_dbg;

    for (auto &p : m_components) {
        delete p.second;
    }

    for (auto &p : m_backends) {
        delete p.second;
    }

    for (auto &p : m_plugins) {
        delete p.second;
    }
}

void PlatformBuilder::create_dbg_init(PlatformParser &parser)
{
    /*
     * To create the debug initiator, we must know which component is the main
     * system bus. The current implemented heuristic takes the only bus of the
     * platform.  If we have multiple buses, this heuristic will need to be
     * updated to know which bus must be considered.
     */

    ParserNode::Subnodes<ParserNodeComponent> buses;

    parser.get_root().find_component_by_attr("tlm-bus", buses);

    if (buses.size() > 1) {
        LOG(APP, ERR) << "Multiple buses in platform is not yet correctly handled. Except failures\n";
        return;
    }

    for (auto & pbus : buses) {
        m_dbg = new DebugInitiator("dbg-initiator", m_config);
        ComponentBase *bus = pbus->get_inst();

        assert(bus != nullptr);
        assert(bus->has_attr("tlm-bus-port"));
        assert(m_dbg->has_attr("tlm-initiator-port"));

        const vector<string> bus_port_name = bus->get_attr("tlm-bus-port");
        const vector<string> dbg_port_name = m_dbg->get_attr("tlm-initiator-port");

        assert(dbg_port_name.size() == 1);

        if (bus_port_name.size() > 1) {
            LOG(APP, WRN) << "Bus component has multiple tlm bus port. Considering the first one.\n";
        }

        LOG(APP, DBG) << "Connecting the debug initiator to the main system bus\n";
        do_binding(bus->get_port(bus_port_name.front()),
                   m_dbg->get_port(dbg_port_name.front()),
                   PlatformDescription::INVALID_DESCRIPTION);
    }
}

void PlatformBuilder::create_plugins(PlatformParser &p)
{
    PluginManager &pm = m_config.get_plugin_manager();

    for (auto &plug : p.get_root().get_plugins()) {
        const string &name = plug.first;
        const string &type = plug.second->get_type();

        PluginManager::Factory p_fact = pm.find_by_type(type);

        assert(p_fact != nullptr);

        LOG(APP, DBG) << "Creating plugin instance `" << name
            << "` of plugin `" << type << "`\n";
        m_plugins[name] = p_fact->create(name, plug.second->get_params());
        plug.second->set_inst(m_plugins[name]);
    }
}

template <class HOOK>
void PlatformBuilder::run_hooks(HOOK &&hook)
{
    for (auto &p : m_plugins) {
        PluginBase *plugin = p.second;

        plugin->hook(hook);
    }
}

void PlatformBuilder::create_components(PlatformParser &parser, CreationStage::value stage)
{
    PlatformDescription::iterator it;
    ComponentManager &cm = m_config.get_component_manager();

    for (auto &comp : parser.get_root().get_components()) {
        const string &name = comp.first;
        const string &type = comp.second->get_type();

        ComponentManager::Factory c_fact = cm.find_by_type(type);

        assert(c_fact != nullptr);

        switch (stage) {
        case CreationStage::DISCOVER:
            c_fact->discover(name, comp.second->get_descr());
            break;

        case CreationStage::CREATE:
            LOG(APP, DBG) << "Creating component " << name
                          << " of type " << type << "\n";
            m_components[name] = c_fact->create(name, comp.second->get_params());
            comp.second->set_inst(m_components[name]);
            break;
        }
    }
}

void PlatformBuilder::create_backends(PlatformParser &p)
{
    BackendManager &pm = m_config.get_backend_manager();

    for (auto &plug : p.get_root().get_backends()) {
        const string &name = plug.first;
        const string &type = plug.second->get_type();

        BackendManager::Factory p_fact = pm.find_by_type(type);

        assert(p_fact != nullptr);

        LOG(APP, DBG) << "Creating backend instance `" << name
            << "` of backend `" << type << "`\n";
        m_backends[name] = p_fact->create(name, plug.second->get_params());
        plug.second->set_inst(m_backends[name]);
    }
}

void PlatformBuilder::do_bindings(PlatformParser &p)
{
    for (auto &comp : p.get_root().get_components()) {
        for (auto &binding : comp.second->get_bindings()) {
            Port &p0 = binding.second->get_local_port();
            Port &p1 = binding.second->get_peer_port();
            PlatformDescription descr = binding.second->get_descr();

            do_binding(p0, p1, descr);
        }
    }
}

void PlatformBuilder::do_binding(Port &p0, Port &p1, PlatformDescription &descr)
{
    LOG(APP, DBG) << "Binding `" << p0.full_name() << "' to `" << p1.full_name() << "'\n";
    if (!p0.connect(p1, descr)) {
        report_parse_warning("Cannot bind `" + p0.full_name() + "' to port `" + p1.full_name() + "'", descr);
        return;
    }
}

void PlatformBuilder::add_component(ComponentBase *c)
{
    const string & name = c->get_name();

    m_parser.get_root().add_component(c);
    m_components[name] = c;
}


void PlatformBuilder::add_backend(ComponentBase *c)
{
    const string & name = c->get_name();

    m_parser.get_root().add_backend(c);
    m_backends[name] = c;
}

void PlatformBuilder::add_plugin(PluginBase *p)
{
    const string & name = p->get_name();

    m_parser.get_root().add_plugin(p);
    m_plugins[name] = p;
}
