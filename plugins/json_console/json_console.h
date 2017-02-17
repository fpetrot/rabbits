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
#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "client.h"

struct SignalGenerator {
    enum Status {
        STA_NEW,
        STA_CREATED,
        STA_FAILURE,
    };

    enum FailureReason {
        FAIL_INVALID_TYPE,
        FAIL_COMP_NOT_FOUND,
        FAIL_PORT_NOT_FOUND,
        FAIL_BINDING,
        FAIL_INTERNAL,
    };

    const std::string name;
    PlatformDescription descr;
    Status m_status = STA_NEW;
    FailureReason m_failure_reason;
    ComponentBase *m_backend = nullptr;

    SignalGenerator(const std::string &name, PlatformDescription &d)
        : name(name)
        , descr(d)
    {}

    void set_failure(FailureReason r) { m_status = STA_FAILURE; m_failure_reason = r; }
};

class JsonConsolePlugin : public Plugin {
public:
    enum SimulationStatus {
        BEFORE_ELABORATION,
        BEFORE_SIMULATION,
        SIMULATION_RUNNING,
        SIMULATION_PAUSED,
        SIMULATION_STOPPED,
        UNKNOWN
    };

private:
    boost::asio::io_service m_asio_service;
    boost::asio::ip::tcp::acceptor m_server_acceptor;

    std::thread m_server_thread;

    std::map<std::string, std::unique_ptr<SignalGenerator> > m_generators;
    int m_cur_generator_idx = 0;

    bool m_wait_before_elaboration = false;
    std::condition_variable m_cv_elaboration;
    std::mutex m_mutex_elaboration;
    bool m_elaboration_done = false;

    void server_entry();
    void client_accept();
    void client_handler_entry();
    void new_client_handler(JsonConsoleClient::Ptr, const boost::system::error_code&);

    void instanciate_backend(const PluginHookAfterBackendInst &hook,
                             std::unique_ptr<SignalGenerator> &gen);

public:
    JsonConsolePlugin(const std::string &name, const Parameters &params, ConfigManager &config);
    virtual ~JsonConsolePlugin();

    void hook(const PluginHookBeforeBuild &);
    void hook(const PluginHookAfterBackendInst &);

    SignalGenerator& create_generator(PlatformDescription &d);

    SimulationStatus get_simulation_status() const;
    void continue_elaboration();
};
