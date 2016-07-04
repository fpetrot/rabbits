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

#include "rabbits/logger/format.h"

#include <unistd.h>
#include <iostream>

#include "rlutil.h"


Formatter::Formatter(std::ostream &o) : m_stream(&o)
{
    detect_tty();
}

Formatter::Formatter(const Formatter &f) : m_stream(f.m_stream), m_is_tty(f.m_is_tty)
{}

Formatter::~Formatter() {}

void Formatter::detect_tty()
{
    /* We compare the streambuf of m_stream with the one of cout and cerr to
     * know if m_stream is effectively stdout or stderr (we assume they haven't
     * be programmatically redirected). We can then check with isatty(3) if we
     * have a terminal. This might not work directly on Windows though. */

    if ((m_stream->rdbuf() == std::cout.rdbuf()) && isatty(1)) {
        m_is_tty = true;
        return;
    }

    if ((m_stream->rdbuf() == std::cerr.rdbuf()) && isatty(2)) {
        m_is_tty = true;
        return;
    }

    m_is_tty = false;
}

void Formatter::set_color(ConsoleColor::value c, ConsoleAttr::value a)
{
    if (!m_is_tty) {
        return;
    }

    int color = int(c) + (8 * int(a));

    rlutil::setColor(color);
}

void Formatter::reset()
{
    rlutil::resetColor();
}

void Formatter::get_tty_attr(int &rows, int &cols) const
{
    if (!m_is_tty) {
        rows = cols = -1;
        return;
    }

    rows = rlutil::trows();
    cols = rlutil::tcols();

}
