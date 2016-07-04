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

#ifndef _RABBITS_LOGGER_FORMAT_H
#define _RABBITS_LOGGER_FORMAT_H

#include <ostream>

struct ConsoleColor
{
    enum value { BLACK = 0, BLUE, GREEN, CYAN, RED, PURPLE, YELLOW, WHITE };
};

struct ConsoleAttr
{
    enum value { NORMAL = 0, BOLD };
};

class Logger;

class Formatter {
protected:
    std::ostream * m_stream;
    bool m_is_tty;

    void detect_tty();

public:
    explicit Formatter(std::ostream &);

    Formatter(const Formatter &f);

    virtual ~Formatter();

    void set_color(ConsoleColor::value c, ConsoleAttr::value a);
    void reset();

    void get_tty_attr(int &rows, int &cols) const;

    bool is_tty() const { return m_is_tty; }
};

#endif
