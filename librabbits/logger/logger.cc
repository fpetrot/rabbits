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

#include <cstdarg>
#include <cstdio>

#include "rabbits/logger.h"
#include "rabbits/config/manager.h"

const std::string Logger::PREFIXES[] = {
        [LogLevel::ERROR]   = "[error]",
        [LogLevel::WARNING] = "[ warn]",
        [LogLevel::INFO]    = "[ info]",
        [LogLevel::DEBUG]   = "[debug]",
        [LogLevel::TRACE]   = "[trace]",
};

const Logger::formater_fn Logger::PREFIX_COLORS[] = {
        [LogLevel::ERROR]   = format::red,
        [LogLevel::WARNING] = format::yellow,
        [LogLevel::INFO]    = format::white,
        [LogLevel::DEBUG]   = format::black,
        [LogLevel::TRACE]   = format::black_b,
};

std::vector<char> Logger::m_format_buf;

char * Logger::vformat(const char * fmt, va_list ap)
{
    va_list aq;

    va_copy(aq, ap);

    if (m_format_buf.capacity() == 0) {
        m_format_buf.resize(DEFAULT_BUF_SIZE);
    }

    size_type capa = m_format_buf.capacity();
    size_type written;

    written = vsnprintf(&m_format_buf[0], m_format_buf.capacity(), fmt, aq);

    while (written >= capa) {
        capa *= 2;
        m_format_buf.resize(capa);

        va_end(aq);
        va_copy(aq, ap);

        written = vsnprintf(&m_format_buf[0], m_format_buf.capacity(), fmt, aq);
    }

    va_end(aq);

    return &m_format_buf[0];
}

char * Logger::format(const char * fmt, ...)
{
    va_list ap;
    char *ret;

    va_start(ap, fmt);
    ret = vformat(fmt, ap);
    va_end(ap);

    return ret;
}


std::ostream & Logger::log_stream(LogLevel::value lvl) const
{
    //if ((lvl > m_level) || m_muted) {
        //return std::cout;
    //}

    //if (m_banner_enabled) {
        //(*m_streams[lvl]) << PREFIXES[lvl];
    //}

    return *m_streams[lvl].sink;
}

void Logger::save_flags()
{
    std::vector< std::ios::fmtflags > flags;

    for (int i = 0; i < LogLevel::LASTLOGLVL; i++) {
        flags.push_back(get_sink(LogLevel::value(i)).flags());
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
        get_sink(LogLevel::value(i)).flags(*it);
    }
}


Logger & get_logger(LogContext::value ctx)
{
    return ConfigManager::get().get_logger(ctx);
}

Logger & get_app_logger()
{
    return ConfigManager::get().get_logger(LogContext::APP);
}

Logger & get_sim_logger()
{
    return ConfigManager::get().get_logger(LogContext::SIM);
}
