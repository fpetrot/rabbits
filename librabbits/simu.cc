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

#include "rabbits/simu.h"

#include "rabbits-common.h"
#include "rabbits/ui/ui.h"

using namespace sc_core;

static pthread_t simu_thread;

void *simu_thread_run(void *arg)
{
    while (sc_get_status() != SC_STOPPED) {
        ui::get_ui()->update();
        sc_start(1, SC_MS);
    }
    ui::get_ui()->stop();

    return NULL;
}

void simu_manager::start()
{
    /* SystemC simulation is started on a separate pthread */
    pthread_create(&simu_thread, NULL, simu_thread_run, NULL);
    pthread_detach(simu_thread);

    ui::get_ui()->event_loop();

    /* in case the event loop ended while SC is running (e.g. window closed) */
    if(sc_core::sc_get_status() != sc_core::SC_STOPPED) {
        sc_core::sc_stop();
    }
}
