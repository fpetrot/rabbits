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

#include <thread>

#include "rabbits/config/manager.h"
#include "rabbits/config/simu.h"

#include "rabbits-common.h"
#include "rabbits/ui/ui.h"

using namespace sc_core;

void SimulationManager::send_event(SimuEvent ev)
{
    for (auto *l: m_pause_listeners) {
        l->simu_event(ev);
    }
}

void SimulationManager::simu_loop()
{
    send_event(SIM_EV_START);
    sc_start();

    while (sc_get_status() == SC_PAUSED) {
        send_event(SIM_EV_PAUSE);
        send_event(SIM_EV_RESUME);
        sc_start();
    }

    send_event(SIM_EV_STOP);
}

void SimulationManager::simu_entry()
{
    LOG(APP, DBG) << "Starting simulation\n";

    install_sig_handlers();

    simu_loop();

    remove_sig_handlers();

    LOG(APP, DBG) << "End of simulation\n";

    if (!m_sysc_stopper.stopped_by_ui()) {
        m_config.get_ui().stop();
    }
}

void SimulationManager::start()
{
    /* SystemC simulation is started on a separate thread as some UI libraries
     * (like Qt) require to run on the main thread */
    std::thread simu_thread(&SimulationManager::simu_entry, this);

    Ui::eExitStatus ui_es;
    ui_es = m_config.get_ui().run();

    LOG(APP, DBG) << "End of UI\n";

    if (ui_es == Ui::WANT_QUIT && sc_get_status() != SC_STOPPED) {
        /* in case the event loop ended while sysc is running
         * (e.g. window closed) */
        LOG(APP, DBG) << "Stopping simulation\n";
        m_sysc_stopper.stop(SystemCStopper::UI);
    }

    LOG(APP, DBG) << "Waiting for simulation to end\n";
    simu_thread.join();

    LOG(APP, DBG) << "Exiting simulation manager\n";
}

void SimulationManager::register_event_listener(SimuEventListener &l)
{
    m_pause_listeners.push_back(&l);
}
