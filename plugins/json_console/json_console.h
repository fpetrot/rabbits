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

#pragma once

#include <rabbits/plugin/plugin.h>
#include <rabbits/config/simu.h>

#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "client.h"
#include "backend.h"

/*
 * Simulation control module.
 * Right now, used to pause the simulation.
 * Once the SimulationManager is able to do that, we can remove this module.
 */
class SimulationControl : public sc_core::sc_module {
private:
    sc_core::sc_time m_period;
    bool m_request;

    void pause_thread()
    {
        for (;;) {
            sc_core::wait(m_period);
            if (m_request) {
                sc_core::sc_pause();
                m_request = false;
            }
        }
    }

public:
    SC_HAS_PROCESS(SimulationControl);

    SimulationControl(sc_core::sc_module_name n, sc_core::sc_time period)
        : sc_core::sc_module(n)
        , m_period(period)
    {
        SC_THREAD(pause_thread);
    }

    void pause_request() { m_request = true; }
};

class JsonConsolePlugin : public Plugin
                        , public SimuPauseListener
                        , public PauseRequestListener {
public:
    enum SimulationStatus {
        BEFORE_ELABORATION = 0,
        BEFORE_SIMULATION,
        SIMULATION_RUNNING,
        SIMULATION_PAUSED,
        SIMULATION_STOPPED,
        UNKNOWN
    };

private:
    typedef std::unique_ptr<BackendInstance> BackendInstancePtr;
    typedef std::pair<const std::string, const std::string> BackendTarget;

    boost::asio::io_service m_asio_service;
    boost::asio::ip::tcp::acceptor m_server_acceptor;

    std::thread m_server_thread;

    std::map<BackendTarget, BackendInstancePtr> m_backends;
    std::map<const std::string, BackendInstance*> m_backends_by_name;

    std::map<const std::string, SignalGenerator::Ptr> m_generators;
    std::map<const std::string, SignalEvent::Ptr> m_events;

    SimulationControl *m_simu_control = nullptr;
    JsonConsoleClient::Ptr m_pause_request_client = nullptr;

    int m_cur_generator_idx = 0;

    bool m_wait_before_elaboration = false;
    bool m_wait_before_simulation = false;
    bool m_pause_request = false;

    std::condition_variable m_cv_elaboration;
    std::mutex m_mutex_elaboration;

    bool m_elaboration_done = false;

    void server_entry();
    void client_accept();
    void client_handler_entry();
    void new_client_handler(JsonConsoleClient::Ptr, const boost::system::error_code&);

    void instanciate_backend(const PluginHookAfterBackendInst &hook,
                             std::unique_ptr<SignalGenerator> &gen);

    BackendInstance * get_backend(PlatformDescription &d);

    void wait_for_client();
    void client_notify();

public:
    JsonConsolePlugin(const std::string &name, const Parameters &params, ConfigManager &config);
    virtual ~JsonConsolePlugin();

    void hook(const PluginHookBeforeBuild &);
    void hook(const PluginHookAfterBackendInst &);
    void hook(const PluginHookAfterBuild &);

    SignalGenerator::Ptr create_generator(PlatformDescription &d);
    SignalEvent::Ptr create_event(PlatformDescription &d, JsonConsoleClient::Ptr);
    const BackendInstance& create_backend(PlatformDescription &d);

    void serialize_backend_val(PlatformDescription &d, PlatformDescription &out);

    bool backend_exists(const std::string & name) const;
    bool generator_exists(const std::string & name) const;
    bool event_exists(const std::string & name) const;

    const BackendInstance & get_backend(const std::string &name) const;
    SignalGenerator::Ptr get_generator(const std::string & name);
    SignalEvent::Ptr get_event(const std::string & name);

    void delete_event(const std::string & name);

    SimulationStatus get_simulation_status() const;
    void continue_elaboration();
    void start_simulation();
    void resume_simulation();
    void pause_simulation(JsonConsoleClient::Ptr);

    /* SimuPauseListener */
    void pause_event();


    /* PauseRequestListener
     * This method is called from the SystemC world to indicate that a pause
     * event is going to happend and that we should handle it. But the
     * sc_pause() call is not to be made by us. */
    void pause_request();
};
