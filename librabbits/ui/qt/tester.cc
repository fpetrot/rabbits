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

#include "rabbits/config.h"
#include "tester.h"

#ifdef RABBITS_CONFIG_POSIX

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <QApplication>

bool QtTester::test()
{
    int pid = fork();

    if (pid == -1) {
        return false;
    }

    if (!pid) {
        int argc = 1;
        char arg0[] = "qt-tester";
        char * argv[] = { arg0 };

        QApplication(argc, argv);

        exit(0);
    } else {
        int wstatus;
        wait(&wstatus);

        return WIFEXITED(wstatus) && (WEXITSTATUS(wstatus) == 0);
    }
}

#else

bool QtTester::test()
{
    return true;
}

#endif
