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
using namespace sc_core;

JsonConsolePlugin::JsonConsolePlugin(const std::string &name,
                                     const Parameters &params,
                                     ConfigManager &config)
    : Plugin(name, params, config)
    , m_server_acceptor(m_asio_service)
{
    m_server_thread = std::thread(&JsonConsolePlugin::server_entry, this);
    m_wait_before_elaboration = params["wait-before-elaboration"].as<bool>();
    m_wait_before_simulation = params["wait-before-simulation"].as<bool>();

    if (m_config.is_simu_manager_available()) {
        m_config.get_simu_manager().register_pause_listener(*this);
    }
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
        unsigned short port = 0;
        bool random_port = m_params["random-port"].as<bool>();

        if (!random_port) {
            port = m_params["port"].as<int>();
        }

        tcp::endpoint endpoint(addr, port);

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

        if (random_port) {
            MLOG(APP, INF) << "listening on TCP port "
                           << m_server_acceptor.local_endpoint().port()
                           << "\n";
        }

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

void JsonConsolePlugin::wait_for_client()
{
    std::unique_lock<std::mutex> lock(m_mutex_elaboration);
    m_cv_elaboration.wait(lock);
}

void JsonConsolePlugin::hook(const PluginHookBeforeBuild &hook)
{
    if (m_wait_before_elaboration) {
        MLOG(APP, INF) << "Waiting for client to send the `continue_elaboration` command...\n";
    }

    while (m_wait_before_elaboration) {
        wait_for_client();
    }

    m_elaboration_done = true;
}

void JsonConsolePlugin::hook(const PluginHookAfterBackendInst &hook)
{
    for (auto & sig: m_backends) {
        sig.second->elaborate(hook.get_builder());
    }

    m_simu_control = new SimulationControl((get_name() + "-simu-control").c_str(),
                                            m_params["max-time-before-pause"].as<sc_time>());
}

void JsonConsolePlugin::hook(const PluginHookAfterBuild &hook)
{
    if (m_wait_before_simulation) {
        MLOG(APP, INF) << "Elaboration done. "
            "Waiting for client to send the `start_simulation` command...\n";
    }

    while (m_wait_before_simulation) {
        wait_for_client();
    }
}

const BackendInstance & JsonConsolePlugin::create_backend(PlatformDescription &d)
{
    const string name = d["component"].as<string>();
    const string port = d["port"].as<string>();
    const string type = d["type"].as<string>();
    const BackendTarget target(name, port);

    if (m_backends.find(target) == m_backends.end()) {
        BackendInstance *inst = new BackendInstance(name, port, type, m_config);
        m_backends[target].reset(inst);
        m_backends_by_name[inst->get_name()] = inst;
    }

    return *m_backends[target];
}

BackendInstance * JsonConsolePlugin::get_backend(PlatformDescription &d)
{
    const string backend_name = d["backend"].as<string>();

    assert(backend_exists(backend_name));

    return m_backends_by_name[backend_name];
}

SignalGenerator::Ptr JsonConsolePlugin::create_generator(PlatformDescription &d)
{
    BackendInstance* backend = get_backend(d);
    SignalGenerator::Ptr gen;

    gen = backend->create_generator(d["params"]);

    m_generators[gen->get_name()] = gen;

    return gen;
}

SignalEvent::Ptr JsonConsolePlugin::create_event(PlatformDescription &d,
                                                 JsonConsoleClient::Ptr client)
{
    BackendInstance* backend = get_backend(d);
    SignalEvent::Ptr ev;

    ev = backend->create_event(d["params"], client, *this);

    m_events[ev->get_name()] = ev;

    return ev;
}

void JsonConsolePlugin::serialize_backend_val(PlatformDescription &d, PlatformDescription &out)
{
    BackendInstance* backend = get_backend(d);
    backend->serialize_val(out);
}

bool JsonConsolePlugin::backend_exists(const std::string & name) const
{
    return m_backends_by_name.find(name) != m_backends_by_name.end();
}

bool JsonConsolePlugin::generator_exists(const std::string & name) const
{
    return m_generators.find(name) != m_generators.end();
}

bool JsonConsolePlugin::event_exists(const std::string & name) const
{
    return m_events.find(name) != m_events.end();
}

const BackendInstance & JsonConsolePlugin::get_backend(const std::string &name) const
{
    assert(backend_exists(name));

    return *m_backends_by_name.at(name);
}

SignalGenerator::Ptr JsonConsolePlugin::get_generator(const std::string & name)
{
    assert(generator_exists(name));

    return m_generators.at(name);
}

SignalEvent::Ptr JsonConsolePlugin::get_event(const std::string & name)
{
    assert(event_exists(name));

    return m_events.at(name);
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

void JsonConsolePlugin::client_notify()
{
    m_cv_elaboration.notify_all();
}

void JsonConsolePlugin::continue_elaboration()
{
    if (m_wait_before_elaboration) {
        m_wait_before_elaboration = false;
        client_notify();
    }
}

void JsonConsolePlugin::start_simulation()
{
    if (m_wait_before_simulation) {
        m_wait_before_simulation = false;
        client_notify();
    }
}

void JsonConsolePlugin::pause_request()
{
    m_pause_request = true;
}

void JsonConsolePlugin::pause_event()
{
    MLOG(APP, INF) << "Simulation paused\n";

    if (m_pause_request_client) {
        m_pause_request_client->pause_event();
    }

    while (m_pause_request) {
        wait_for_client();
    }
}

void JsonConsolePlugin::resume_simulation()
{
    if (m_pause_request) {
        m_pause_request = false;
        client_notify();
    }
}

void JsonConsolePlugin::pause_simulation(JsonConsoleClient::Ptr c)
{
    assert(m_simu_control);
    m_pause_request_client = c;
    m_simu_control->pause_request(); /* The effective sc_pause() call */
    pause_request(); /* We will handle the pause event */
}

