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

#include "rabbits/logger.h"

#include <cstdarg>
#include <cstdio>
const std::string Logger::PREFIXES[] = {
        [LogLevel::ERROR]   = "[error] ",
        [LogLevel::WARNING] = "[ warn] ",
        [LogLevel::INFO]    = "[ info] ",
        [LogLevel::DEBUG]   = "[debug] ",
};

Logger Logger::m_logger;

int Logger::log_vprintf(LogLevel::value lvl, const std::string fmt, va_list ap) const
{
    va_list aq;

    va_copy(aq, ap);

    size_type capa = m_format_buf.capacity();
    size_type written;

    written = vsnprintf(&m_format_buf[0], m_format_buf.capacity(), fmt.c_str(), aq);

    while (written >= capa) {
        capa *= 2;
        m_format_buf.resize(capa);

        va_end(aq);
        va_copy(aq, ap);
        
        written = vsnprintf(&m_format_buf[0], m_format_buf.capacity(), fmt.c_str(), aq);
    }

    va_end(aq);
    log_stream(lvl) << std::string(&m_format_buf[0]);

    return written;
}

int Logger::log_printf(LogLevel::value lvl, const std::string fmt, ...) const
{
    va_list ap;
    size_type written;

    va_start(ap, fmt);
    written = log_vprintf(lvl, fmt, ap);
    va_end(ap);

    return written;
}


std::ostream & Logger::log_stream(LogLevel::value lvl) const
{
    if (lvl > m_level) {
        return m_null_stream;
    }

    return (*m_streams[lvl]) << PREFIXES[lvl];
}

void Logger::save_flags()
{
    std::vector< std::ios::fmtflags > flags;

    for (int i = 0; i < LogLevel::LASTLOGLVL; i++) {
        flags.push_back(m_streams[i]->flags());
    }

    m_state_stack.push(flags);
}

void Logger::restore_flags()
{
    std::vector< std::ios::fmtflags >::iterator it;
    int i;

    if (m_state_stack.empty()) {
        return;
    }

    std::vector< std::ios::fmtflags > &flags = m_state_stack.top();
    m_state_stack.pop();

    for (it = flags.begin(), i = 0; it != flags.end(); it++, i++) {
        m_streams[i]->flags(*it);
    }
}
