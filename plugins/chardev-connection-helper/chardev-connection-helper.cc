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
#include <boost/algorithm/string.hpp>

#include "chardev-connection-helper.h"
#include "rabbits/platform/parser.h"
#include "rabbits/platform/builder.h"
#include "rabbits/config/manager.h"

using std::vector;
using std::string;
using std::stringstream;

bool CharDevConnectionHelperPlugin::stdio_is_locked(PlatformParser &p)
{
    if (m_stdio_locked) {
        return true;
    }

    ParserNode::Subnodes<ParserNodeBackend> out;

    p.get_root().find_backend_by_attr("stdio-locked", out);

    if (out.size()) {
        m_stdio_locked = true;
    }

    return m_stdio_locked;
}

string CharDevConnectionHelperPlugin::get_param(const string comp) const
{
    return "connect-" + comp + "-to";
}

string CharDevConnectionHelperPlugin::gen_unique_name(const string &type)
{
    stringstream ss;

    ss << get_name() << "-auto-" << type << "-" << m_unique_idx;
    m_unique_idx++;

    return ss.str();
}

void CharDevConnectionHelperPlugin::create_params(const PluginHookAfterComponentInst &h)
{
    for (auto &node : m_char_nodes) {
        const string param = get_param(node->get_name());
        const string descr = "Connect the character port of component `" + node->get_name()
            + "` to a character backend "
            "(valid values are `null`, `stdio`, `serial,/path/to/tty`, `socket,tcp|udp|unix,address[,server][,nowait]`)";

        m_params.add(param, Parameter<string>(descr, ""));
        h.get_builder().get_config().add_param_alias(param, m_params[param]);
    }
}

void CharDevConnectionHelperPlugin::parse_params(const PluginHookAfterComponentInst &h)
{
    for (auto &node : m_char_nodes) {
        const string param = get_param(node->get_name());

        if (m_params[param].is_default()) {
            continue;
        }

        const string val = m_params[param].as<string>();

        string type;
        std::vector<string> strs;
        boost::split(strs, val, boost::is_any_of(","));

        string arg0 = strs[0];
        if (arg0 == "null") {
            type = "chardev-null";
        } else if (arg0 == "stdio") {
            type = "chardev-stdio";
        } else if (arg0 == "serial") {
            type = "chardev-serial";
        } else if(arg0 == "socket") {
            type = "chardev-socket";
        } else {
            MLOG(APP, ERR) << "Invalid value `" << val << "` for parameter `-"
                << param << "`. Ignoring. See -help\n";
            continue;
        }

        MLOG(APP, TRC) << "Connecting component `" << node->get_name()
            << "` to a " << type << "\n";

        BackendManager &bm = h.get_builder().get_config().get_backend_manager();
        BackendManager::Factory f = bm.find_by_type(type);
        if (f == nullptr) {
            MLOG(APP, ERR) << type << " backend is unavailable. Broken Rabbits installation?\n";
            continue;
        }

        Parameters b_params = f->get_params();
        if(arg0 == "serial") {
            if(strs.size() > 1) {
                b_params["path"].set(strs[1]);
            }
        }
        else if(arg0 == "socket") {
            if(strs.size() >= 3) {
                b_params["kind"].set(strs[1]);
                b_params["address"].set(strs[2]);
            }
            if(strs.size() >= 4) {
                bool server = strs[3] == "server";
                b_params["server"].set(server);
            }
            if(strs.size() >= 5) {
                bool nowait = strs[3] == "nowait" || strs[4] == "nowait";
                b_params["nowait"].set(nowait);
            }
        }

        const string name = gen_unique_name(type);

        std::shared_ptr<ParserNodeBackend> b
            = h.get_parser().get_root().create_backend(name, type, b_params);

        const string port_s = node->get_inst()->get_attr("char-port").front();

        MLOG(APP, TRC) << "Adding binding " << node->get_name() << "." << port_s
            << " -> " << name << ".(default port) to parser\n";

        node->remove_binding_if_exists(port_s);
        node->add_binding(port_s, b, "", PlatformDescription::INVALID_DESCRIPTION);
    }
}

void CharDevConnectionHelperPlugin::hook(const PluginHookAfterComponentInst &h)
{
    PlatformParser &parser = h.get_parser();
    parser.get_root().find_component_by_attr("char-port", m_char_nodes);

    MLOG(APP, TRC) << "Found " << m_char_nodes.size() << " component(s) with char ports\n";

    create_params(h);
    parse_params(h);
}

void CharDevConnectionHelperPlugin::autoconnect(const PluginHookAfterBuild &h, Port &p, bool to_stdio)
{
    if (p.is_connected()) {
        MLOG(APP, TRC) << "Port " << p.full_name() << " already connected. Skipping.\n";
        return;
    }

    ComponentBase *backend = nullptr;
    BackendManager &bm = h.get_builder().get_config().get_backend_manager();

    if (to_stdio && (!stdio_is_locked(h.get_parser()))) {
        MLOG(APP, DBG) << "Auto-connecting " << p.full_name() << " to a chardev-stdio instance\n";
        BackendManager::Factory f = bm.find_by_type("chardev-stdio");

        if (f == nullptr) {
            MLOG(APP, ERR) << "chardev-stdio backend is unavailable. Broken Rabbits installation?\n";
            return;
        }

        backend = f->create("conn-helper-auto-chardev-stdio", PlatformDescription::INVALID_DESCRIPTION);
        m_stdio_locked = true;

    } else {
        MLOG(APP, DBG) << "Auto-connecting " << p.full_name() << " to a chardev-null instance\n";

        BackendManager::Factory f = bm.find_by_type("chardev-null");

        if (f == nullptr) {
            MLOG(APP, ERR) << "chardev-null backend is unavailable. Broken Rabbits installation?\n";
            return;
        }

        backend = f->create(gen_unique_name("chardev-null"),
                            PlatformDescription::INVALID_DESCRIPTION);
    }

    h.get_builder().add_backend(backend);
    const vector<string> char_ports = backend->get_attr("char-port");

    p.connect(backend->get_port(char_ports.front()), PlatformDescription::INVALID_DESCRIPTION);
}

void CharDevConnectionHelperPlugin::autoconnect(const PluginHookAfterBuild &h, ComponentBase &comp, bool to_stdio)
{
    const vector<string> char_ports = comp.get_attr("char-port");

    MLOG(APP, TRC) << "Module " << comp.get_full_name() << " has "
        << char_ports.size() << " char port(s)\n";

    for (auto &port_name : char_ports) {
        autoconnect(h, comp.get_port(port_name), to_stdio);
    }
}

void CharDevConnectionHelperPlugin::autoconnect(const PluginHookAfterBuild &h, ParserNodeComponent &node)
{
    ComponentBase *comp = node.get_inst();
    assert(comp != nullptr);

    autoconnect(h, *comp, true);
}

void CharDevConnectionHelperPlugin::autoconnect(const PluginHookAfterBuild &h, ParserNodeBackend &node)
{
    ComponentBase *comp = node.get_inst();
    assert(comp != nullptr);

    autoconnect(h, *comp, false);
}

void CharDevConnectionHelperPlugin::hook(const PluginHookAfterBuild &h)
{
    for (auto &node : m_char_nodes) {
        autoconnect(h, *node);
    }

    for (auto &node : h.get_parser().get_root().get_backends()) {
        autoconnect(h, *(node.second));
    }
}
