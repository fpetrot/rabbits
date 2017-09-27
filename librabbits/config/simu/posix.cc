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

#include <signal.h>
#include <cassert>

#include "rabbits/logger.h"
#include "rabbits/config/simu.h"

static SystemCStopper *sysc_stopper = nullptr;
static struct sigaction previous_sigaction;

static void term_handler(int signal)
{
    assert(sysc_stopper);

    sysc_stopper->stop(SystemCStopper::OTHER);
}

void SimulationManager::install_sig_handlers()
{
    if (sysc_stopper) {
        LOG(APP, ERR) << "Internal error: " << __func__ << " has already been called?\n";
        return;
    }

    struct sigaction s;
    std::memset(&s, 0, sizeof(s));

    sysc_stopper = &m_sysc_stopper;

    s.sa_handler = term_handler;
    sigaction(SIGINT, &s, &previous_sigaction);
}

void SimulationManager::remove_sig_handlers()
{
    sigaction(SIGINT, &previous_sigaction, NULL);

    sysc_stopper = nullptr;
}

