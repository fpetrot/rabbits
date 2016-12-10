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

#include "rabbits/simu.h"

#include "rabbits-common.h"
#include "rabbits/ui/ui.h"

using namespace sc_core;

/* 
 * SystemC simulator is not thread safe. We can't call sc_stop() from a thread
 * that didn't call sc_start(). While waiting for better solution, we use this
 * module to stop the simulation.
 */
class SystemCStopper : public sc_module {
protected:
    bool m_run = true;

    void stop_thread() {
        for (;;) {
            wait(100, SC_MS);
            if (!m_run) {
                sc_stop();
            }
        }
    }

public:
    SC_HAS_PROCESS(SystemCStopper);
    SystemCStopper(sc_module_name n)
        : sc_module(n)
    {
        SC_THREAD(stop_thread);
    }

    void stop() 
    {
        m_run = false;
    }

    bool stopped_by_ui() const
    {
        return !m_run;
    }
};

static void simu_thread_entry(SystemCStopper *sysc_stopper, ConfigManager *config)
{
    LOG(APP, DBG) << "Starting simulation\n";
    sc_start();
    LOG(APP, DBG) << "End of simulation\n";

    if (!sysc_stopper->stopped_by_ui()) {
        config->get_ui().stop();
    }
}

void SimuManager::start()
{
    SystemCStopper sysc_stopper("rabbits-simulation-helper");

    /* SystemC simulation is started on a separate thread as some UI libraries
     * (like Qt) require to run on the main thread */
    std::thread simu_thread(simu_thread_entry, &sysc_stopper, &m_config);

    Ui::eExitStatus ui_es;
    ui_es = m_config.get_ui().run();

    LOG(APP, DBG) << "End of UI\n";

    switch(ui_es) {
    case Ui::WANT_QUIT:
        /* in case the event loop ended while sysc is running
         * (e.g. window closed) */
        if(sc_get_status() != SC_STOPPED) {
            LOG(APP, DBG) << "Stopping simulation\n";
            sysc_stopper.stop();
        }

        /* Fallthrough */
    case Ui::CONTINUE:
        LOG(APP, DBG) << "Waiting for simulation to end\n";
        simu_thread.join();
    }

    LOG(APP, DBG) << "Exiting simulation manager\n";
}
