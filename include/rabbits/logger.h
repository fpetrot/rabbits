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

#ifndef _UTILS_LOGGER_H
#define _UTILS_LOGGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <stack>

#include "rabbits/config.h"

class Logger {
public:
    enum LogLevel {
        ERROR = 0, WARNING, INFO, DEBUG,
        LASTLOGLVL
    };

    static const std::string PREFIXES[];

protected:
    typedef std::vector<char>::size_type size_type;
    static const size_type DEFAULT_BUF_SIZE = 256;

private:
    static Logger m_logger;

    Logger() : m_level(LogLevel(RABBITS_LOGLEVEL))  {
        for (int i = 0; i < LASTLOGLVL; i++) {
            m_streams[i] = &std::cerr;
        }

        m_format_buf.resize(DEFAULT_BUF_SIZE);
    }

    Logger(const Logger&);
    Logger& operator=(const Logger&);

protected:
    LogLevel m_level;
    std::ostream * m_streams[LASTLOGLVL];
    std::ofstream m_null_stream;

    typedef std::stack<std::vector<std::ios::fmtflags> > StateStack;
    StateStack m_state_stack;

    std::vector<char> m_format_buf;

public:
    static Logger & get() { return m_logger; }

    void set_stream(LogLevel lvl, std::ostream *str) { m_streams[lvl] = str; }
    void set_log_level(LogLevel lvl) { m_level = lvl; }

    int printf(LogLevel lvl, const std::string fmt, ...);
    std::ostream & stream(LogLevel lvl);

    void save_flags();
    void restore_flags();
};

#ifndef RABBITS_LOGLEVEL
# define RABBITS_LOGLEVEL 0
#endif

#ifdef VERBOSE_TRACES

# define TRACE_PRINTF(lvl, fmt, ...) \
    Logger::get().printf(lvl, "|%s| " fmt, __PRETTY_FUNCTION__, ##__VA_ARGS__)
# define TRACE_STREAM(lvl, ...) \
    Logger::get().stream(lvl) << "|" << __PRETTY_FUNCTION__ << "| " << __VA_ARGS__

#else

# define TRACE_PRINTF(lvl, fmt, ...) \
    Logger::get().printf(lvl, fmt, ##__VA_ARGS__)
# define TRACE_STREAM(lvl, ...) \
    Logger::get().stream(lvl) << __VA_ARGS__

#endif



#define ERR_PRINTF(fmt, ...) TRACE_PRINTF(Logger::ERROR, fmt, ##__VA_ARGS__)
#define ERR_STREAM(...)      TRACE_STREAM(Logger::ERROR, ##__VA_ARGS__)

#if (RABBITS_LOGLEVEL > 0)
# define WRN_PRINTF(fmt, ...) TRACE_PRINTF(Logger::WARNING, fmt, ##__VA_ARGS__)
# define WRN_STREAM(...)      TRACE_STREAM(Logger::WARNING, ##__VA_ARGS__)
#else
# define WRN_PRINTF(fmt, ...) do {} while(0)
# define WRN_STREAM(...)      do {} while(0)
#endif

#if (RABBITS_LOGLEVEL > 1)
# define INF_PRINTF(fmt, ...) TRACE_PRINTF(Logger::INFO, fmt, ##__VA_ARGS__)
# define INF_STREAM(...)      TRACE_STREAM(Logger::INFO, ##__VA_ARGS__)
#else
# define INF_PRINTF(fmt, ...) do {} while(0)
# define INF_STREAM(...)      do {} while(0)
#endif

#if (RABBITS_LOGLEVEL > 2)
# define DBG_PRINTF(fmt, ...) TRACE_PRINTF(Logger::DEBUG, fmt, ##__VA_ARGS__)
# define DBG_STREAM(...)      TRACE_STREAM(Logger::DEBUG, ##__VA_ARGS__)
#else
# define DBG_PRINTF(fmt, ...) do {} while(0)
# define DBG_STREAM(...)      do {} while(0)
#endif

#endif
