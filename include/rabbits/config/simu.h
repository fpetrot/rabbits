/*
 *  This file is part of Rabbits
 *  Copyright (C) 2015-2017  Clement Deschamps and Luc Michel
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

#ifndef _UTILS_SIMU_MANAGER_H
#define _UTILS_SIMU_MANAGER_H

#include <vector>

#include <systemc>

enum SimuEvent {
    SIM_EV_START,
    SIM_EV_PAUSE,
    SIM_EV_RESUME,
    SIM_EV_STOP,
};

class SimuEventListener {
public:
    virtual void simu_event(SimuEvent) = 0;
};

/*
 * SystemC simulator is not thread safe. We can't call sc_stop() from a thread
 * that didn't call sc_start(). While waiting for better solution, we use this
 * module to stop the simulation.
 */
class SystemCStopper : public sc_core::sc_module {
public:
    enum eStopSource {
        UI, OTHER
    };

protected:
    bool m_run = true;
    eStopSource m_source;

    void stop_thread() {
        for (;;) {
            wait(100, sc_core::SC_MS);
            if (!m_run) {
                sc_core::sc_stop();
            }
        }
    }

public:
    SC_HAS_PROCESS(SystemCStopper);
    SystemCStopper(sc_core::sc_module_name n)
        : sc_module(n)
    {
        SC_THREAD(stop_thread);
    }

    void stop(eStopSource source)
    {
        if (m_run) {
            m_run = false;
            m_source = source;
        }
    }

    bool stopped_by_ui() const
    {
        return (!m_run) && (m_source == UI);
    }
};

class ConfigManager;

class SimulationManager
{
private:
    ConfigManager &m_config;

    SystemCStopper m_sysc_stopper;

    std::vector<SimuEventListener*> m_pause_listeners;

    void simu_loop();
    void simu_entry();

    void install_sig_handlers();
    void remove_sig_handlers();

    void send_event(SimuEvent ev);

public:
    SimulationManager(ConfigManager &config)
        : m_config(config)
        , m_sysc_stopper("rabbits-simulation-helper")
    {}

    virtual ~SimulationManager() {}

    void start();

    void register_event_listener(SimuEventListener &);
};

#endif
