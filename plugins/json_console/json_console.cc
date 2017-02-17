/*
 *  This file is part of Rabbits
 *  Copyright (C) 2017  Clement Deschamps and Luc Michel
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

#include <chrono>
#include <boost/bind.hpp>

#include <systemc>

#include <rabbits/platform/description.h>
#include <rabbits/platform/builder.h>

#include "json_console.h"

using std::string;
using namespace boost::asio::ip;

JsonConsolePlugin::JsonConsolePlugin(const std::string &name,
                                     const Parameters &params,
                                     ConfigManager &config)
    : Plugin(name, params, config)
    , m_server_acceptor(m_asio_service)
{
    m_server_thread = std::thread(&JsonConsolePlugin::server_entry, this);
    m_wait_before_elaboration = params["wait-before-elaboration"].as<bool>();
}

JsonConsolePlugin::~JsonConsolePlugin()
{
    m_asio_service.stop();
    m_server_thread.join();
}

void JsonConsolePlugin::server_entry()
{
    try {
        address addr = address::from_string("::");
        tcp::endpoint endpoint(addr, m_params["port"].as<int>());

        m_server_acceptor.open(endpoint.protocol());

        if (endpoint.protocol() == tcp::v6()) {
            boost::system::error_code err;
            m_server_acceptor.set_option(v6_only(false), err);

            if (err) {
                MLOG(APP, ERR) << "Cannot start TCP server: " << err << "\n";
                return;
            }
        }

        boost::asio::ip::tcp::acceptor::reuse_address option(true);
        m_server_acceptor.set_option(option);
        m_server_acceptor.bind(endpoint);
        m_server_acceptor.listen();

        client_accept();
        m_asio_service.run();

        m_server_acceptor.close();

    } catch (std::exception &e) {
        MLOG(APP, ERR) << "Died: " << e.what() << "\n";
    }
}

void JsonConsolePlugin::client_accept()
{
    JsonConsoleClient::Ptr client(new JsonConsoleClient(*this, m_asio_service));

    m_server_acceptor.async_accept(client->get_socket(),
                                   boost::bind(&JsonConsolePlugin::new_client_handler, this,
                                               client, boost::asio::placeholders::error));
}

void JsonConsolePlugin::new_client_handler(JsonConsoleClient::Ptr client,
                                           const boost::system::error_code &err)
{
    MLOG(APP, TRC) << "New client " << client->get_id() << "\n";
    client->wait_for_cmd();
    client_accept();
}

void JsonConsolePlugin::hook(const PluginHookBeforeBuild &hook)
{
    while (m_wait_before_elaboration) {
        MLOG(APP, INF) << "Waiting for client to send the `continue_elaboration` command...\n";
        std::unique_lock<std::mutex> lock(m_mutex_elaboration);
        m_cv_elaboration.wait(lock);
    }

    m_elaboration_done = true;
}

void JsonConsolePlugin::instanciate_backend(const PluginHookAfterBackendInst &hook,
                                            std::unique_ptr<SignalGenerator> &gen)
{
    PlatformBuilder &builder = hook.get_builder();
    BackendManager &bm = builder.get_config().get_backend_manager();
    const string backend_type = gen->descr["type"].as<string>();
    const char * type_name;

    BackendManager::Factory f;

    if (backend_type == "bool") {
        type_name = "stub-bool";
    } else if (backend_type == "double") {
        type_name = "stub-double";
    } else {
        MLOG(APP, DBG) << "Invalid type `" << backend_type << "` for `" << gen->name << "`\n";
        gen->set_failure(SignalGenerator::FAIL_INVALID_TYPE);
        return;
    }

    if (!bm.type_exists(type_name)) {
        MLOG(APP, WRN) << "Backend `" << type_name << "` not found. Broken Rabbits installation?\n";
        gen->set_failure(SignalGenerator::FAIL_INTERNAL);
        return;
    }

    f = bm.find_by_type(type_name);

    std::stringstream ss;
    ss << get_name() << "-autogen-" << gen->name;

    ComponentBase *c = f->create(ss.str(), gen->descr["params"]);

    if (!c) {
        MLOG(APP, WRN) << "Cannot create backend `" << ss.str() << "`.\n";
        gen->set_failure(SignalGenerator::FAIL_INTERNAL);
        return;
    }

    builder.add_backend(c);

    const string stubed_component = gen->descr["component"].as<string>();
    const string stubed_port = gen->descr["port"].as<string>();

    if (!builder.comp_exists(Namespace::get(Namespace::COMPONENT), stubed_component)) {
        MLOG(APP, DBG) << "Component `" << stubed_component
                       << "` not found for `" << gen->name << "`\n";

        gen->set_failure(SignalGenerator::FAIL_COMP_NOT_FOUND);
        return;
    }

    ComponentBase &stubed = builder.get_comp(Namespace::get(Namespace::COMPONENT),
                                             stubed_component);

    if (!stubed.port_exists(stubed_port)) {
        MLOG(APP, DBG) << "Port `" << stubed_port << "` on component `" << stubed_component
                       << "` not found for `" << gen->name << "`\n";

        gen->set_failure(SignalGenerator::FAIL_PORT_NOT_FOUND);
        return;
    }

    MLOG(APP, DBG) << "Binding " << c->get_name() << "." << "port -> "
                   << stubed.get_name() << "." << stubed_port << "\n";

    if(!stubed.get_port(stubed_port).connect(c->get_port("port"),
                                             PlatformDescription::INVALID_DESCRIPTION)) {
        MLOG(APP, DBG) << "Binding failed for `" << gen->name << "`\n";
        gen->set_failure(SignalGenerator::FAIL_BINDING);
    }
}

void JsonConsolePlugin::hook(const PluginHookAfterBackendInst &hook)
{
    for (auto &gen: m_generators) {
        instanciate_backend(hook, gen.second);
    }
}

SignalGenerator & JsonConsolePlugin::create_generator(PlatformDescription &d)
{
    std::stringstream ss;

    ss << "generator" << m_cur_generator_idx++;

    SignalGenerator * gen = new SignalGenerator(ss.str(), d);

    m_generators[gen->name].reset(gen);

    return *gen;
}

JsonConsolePlugin::SimulationStatus JsonConsolePlugin::get_simulation_status() const
{
    if (!m_elaboration_done) {
        return BEFORE_ELABORATION;
    } else {
        switch (sc_core::sc_get_status()) {
        case sc_core::SC_ELABORATION:
        case sc_core::SC_BEFORE_END_OF_ELABORATION:
        case sc_core::SC_END_OF_ELABORATION:
            return BEFORE_SIMULATION;

        case sc_core::SC_START_OF_SIMULATION:
        case sc_core::SC_RUNNING:
            return SIMULATION_RUNNING;

        case sc_core::SC_PAUSED:
            return SIMULATION_PAUSED;

        case sc_core::SC_STOPPED:
        case sc_core::SC_END_OF_SIMULATION:
            return SIMULATION_STOPPED;

        default:
            return UNKNOWN;
        }
    }
}

void JsonConsolePlugin::continue_elaboration()
{
    m_wait_before_elaboration = false;
    m_cv_elaboration.notify_all();
}
