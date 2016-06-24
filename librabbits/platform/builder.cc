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

#include "rabbits/plugin/hook.h"
#include "rabbits/plugin/manager.h"

using namespace sc_core;
using std::string;
using std::stringstream;
using std::vector;

static inline void report_parse_warning(const string &msg, const PlatformDescription &descr)
{
    WRN_STREAM(msg << " (at " << descr.origin() << ")\n");
}

static void tokenize(const string s, vector<string>& toks, const char sep = '.')
{
    std::istringstream ss(s);
    string tok;

    while(std::getline(ss, tok, sep)) {
        toks.push_back(tok);
    }
}

PlatformBuilder::PlatformBuilder(sc_module_name name, PlatformDescription &descr)
    : sc_module(name), m_dbg(NULL)
{
    PluginManager &pm = PluginManager::get();

    pm.run_hook(PluginHookBeforeBuild(&descr, this));

    create_components(descr, CreationStage::DISCOVER);
    pm.run_hook(PluginHookAfterComponentDiscovery(&descr, this));
    
    create_components(descr, CreationStage::CREATE);
    pm.run_hook(PluginHookAfterComponentInst(&descr, this));
    
    do_bindings(descr);
    pm.run_hook(PluginHookAfterBusConnections(&descr, this));

    create_dbg_init();

    pm.run_hook(PluginHookAfterBuild(&descr, this));
}

PlatformBuilder::~PlatformBuilder()
{}

void PlatformBuilder::create_dbg_init()
{
    /*
     * To create the debug initiator, we must know which component is the main
     * system bus. The current implemented heuristic takes the only bus of the
     * platform.  If we have multiple buses, this heuristic will need to be
     * updated to know which bus must be considered.
     */

    vector<ComponentBase*> buses;

    find_comp_by_attr("tlm-bus", buses);

    if (buses.size() > 1) {
        ERR_STREAM("Multiple buses in platform is not yet correctly handled. Except failures\n");
        return;
    }

    for (ComponentBase* bus : buses) {
        m_dbg = new DebugInitiator("dbg-initiator");

        assert(bus->has_attr("tlm-bus-port"));
        assert(m_dbg->has_attr("tlm-initiator-port"));

        const string &bus_port_name = bus->get_attr("tlm-bus-port");
        const string &dbg_port_name = m_dbg->get_attr("tlm-initiator-port");

        DBG_STREAM("Connecting the debug initiator to the main system bus\n");
        do_binding(bus->get_port(bus_port_name),
                   m_dbg->get_port(dbg_port_name),
                   PlatformDescription::INVALID_DESCRIPTION);
    }
}

void PlatformBuilder::create_components(PlatformDescription &descr, CreationStage::value stage)
{
    PlatformDescription::iterator it;
    ComponentManager &cm = ComponentManager::get();

    if ((!descr.exists("components")) || (descr["components"].type() != PlatformDescription::MAP)) {
        DBG_STREAM("No component found in description\n");
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

void PlatformBuilder::find_comp_by_attr(const std::string &key, std::vector<ComponentBase*> &out)
{
    comp_iterator it;

    for (it = comp_begin(); it != comp_end(); it++) {
        if (it->second->has_attr(key)) {
            out.push_back(it->second);
        }
    }
}

void PlatformBuilder::do_bindings(PlatformDescription &descr)
{
    PlatformDescription::iterator it;

    if (!descr["components"].is_map()) {
        return;
    }

    for (it = descr["components"].begin(); it != descr["components"].end(); it++) {
        if (m_components.find(it->first) == m_components.end()) {
            continue;
        }

        do_bindings(*m_components[it->first], it->second);
    }
}

void PlatformBuilder::do_bindings(ComponentBase &c, PlatformDescription &descr)
{
    PlatformDescription::iterator it;

    if (!descr["bindings"].is_map()) {
        return;
    }

    for (it = descr["bindings"].begin(); it != descr["bindings"].end(); it++) {
        const string &pname = it->first;

        if (!c.port_exists(pname)) {
            report_parse_warning("Port `" + pname + "' in component `" + c.name() + "' not found", it->second);
            continue;
        }

        do_binding(c.get_port(pname), it->second);
    }
}

class ParsedPeer {
private:
    string m_name_space;
    string m_name;
    string m_port;

    bool name_ok, port_ok, name_space_ok;

public:
    ParsedPeer() : name_ok(false), port_ok(false), name_space_ok(false) {}

    void set_name(const string &n) { m_name = n; name_ok = true; }
    void set_name_space(const string &n) { m_name_space = n; name_space_ok = true; }
    void set_port(const string &n) { m_port = n; port_ok = true; }

    bool is_valid() { return name_ok && port_ok; }
    bool has_name() { return name_ok; }
    bool has_port() { return port_ok; }

    const string & name() { return m_name; }
    const string & name_space() { return m_name_space; }
    const string & port() { return m_port; }
};

static bool parse_peer_name(const string name, ParsedPeer &peer)
{
    vector<string> toks;
    tokenize(name, toks);

    switch(toks.size()) {
    case 3:
        /* namespace.component.port */
        report_parse_warning("n/i", PlatformDescription());
        return false;

    case 2:
        /* component.port (current namespace) */
        peer.set_name(toks[0]);
        peer.set_port(toks[1]);
        break;

    case 1:
        /* component (current namespace, implicit port) */
        peer.set_name(toks[0]);
        break;

    default:
        return false;
    }

    return true;
}

void PlatformBuilder::do_binding(Port &p, PlatformDescription &descr)
{
    ParsedPeer peer;

    switch (descr.type()) {
    case PlatformDescription::MAP:
        if (descr["peer"].is_scalar()) {
            if (!parse_peer_name(descr["peer"].as<string>(), peer)) {
                report_parse_warning("Invalid peer binding description", descr);
                return;
            }
        }

        if (descr["port"].is_scalar()) {
            peer.set_port(descr["port"].as<string>());
        }
        break;

    case PlatformDescription::SCALAR:
        if (!parse_peer_name(descr.as<string>(), peer)) {
            report_parse_warning("Invalid peer binding description", descr);
            return;
        }
        break;

    default:
        report_parse_warning("Invalid binding description for port `" + p.name() + "'", descr);
        return;
    }

    if (!peer.has_name()) {
        report_parse_warning("Invalid peer binding description", descr);
        return;
    }

    if (!comp_exists(peer.name())) {
        report_parse_warning("Peer component `" + peer.name() + "' not found", descr);
        return;
    }

    ComponentBase &peer_comp = get_comp(peer.name());

    if (!peer.has_port()) {
        /* When no port is specified, we take the first available port */
        if (peer_comp.port_exists(0)) {
            peer.set_port(peer_comp.get_port(0).name());
        } else {
            report_parse_warning("No port found on component `" + peer.name() + "'", descr);
            return;
        }
    }

    if (!peer_comp.port_exists(peer.port())) {
        report_parse_warning("Port `" + peer.port() + "' on peer component `" + peer.name() + "' not found", descr);
        return;
    }

    Port &peer_port = peer_comp.get_port(peer.port());

    do_binding(p, peer_port, descr);
}

void PlatformBuilder::do_binding(Port &p0, Port &p1, PlatformDescription &descr)
{
    DBG_STREAM("Binding `" << p0.full_name() << "' to `" << p1.full_name() << "'\n");
    if (!p0.connect(p1, descr)) {
        report_parse_warning("Cannot bind `" + p0.full_name() + "' to port `" + p1.full_name() + "'", descr);
        return;
    }
}

