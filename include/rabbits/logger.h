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
#include "logger/logger.h"

static inline std::ostream & log_stream(LogLevel::value lvl)
{
    return Logger::get().log_stream(lvl);
}

static inline int log_vprintf(LogLevel::value lvl, const std::string fmt,
                              va_list ap)
{
    return Logger::get().log_vprintf(lvl, fmt, ap);
}

static inline int log_printf(LogLevel::value lvl, const std::string fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = log_vprintf(lvl, fmt, ap);
    va_end(ap);

    return ret;
}

#ifndef RABBITS_LOGLEVEL
# define RABBITS_LOGLEVEL 0
#endif

#ifdef VERBOSE_TRACES

# define TRACE_PRINTF(lvl, fmt, ...) \
    log_printf(lvl, "|%s| " fmt, __PRETTY_FUNCTION__, ##__VA_ARGS__)
# define TRACE_STREAM(lvl, ...) \
    log_stream(lvl) << "|" << __PRETTY_FUNCTION__ << "| " << __VA_ARGS__

#else

# define TRACE_PRINTF(lvl, fmt, ...) \
    log_printf(lvl, fmt, ##__VA_ARGS__)
# define TRACE_STREAM(lvl, ...) \
    log_stream(lvl) << __VA_ARGS__

#endif



#define ERR_PRINTF(fmt, ...) TRACE_PRINTF(LogLevel::ERROR, fmt, ##__VA_ARGS__)
#define ERR_STREAM(...)      TRACE_STREAM(LogLevel::ERROR, ##__VA_ARGS__)

#if (RABBITS_LOGLEVEL > 0)
# define WRN_PRINTF(fmt, ...) TRACE_PRINTF(LogLevel::WARNING, fmt, ##__VA_ARGS__)
# define WRN_STREAM(...)      TRACE_STREAM(LogLevel::WARNING, ##__VA_ARGS__)
#else
# define WRN_PRINTF(fmt, ...) do {} while(0)
# define WRN_STREAM(...)      do {} while(0)
#endif

#if (RABBITS_LOGLEVEL > 1)
# define INF_PRINTF(fmt, ...) TRACE_PRINTF(LogLevel::INFO, fmt, ##__VA_ARGS__)
# define INF_STREAM(...)      TRACE_STREAM(LogLevel::INFO, ##__VA_ARGS__)
#else
# define INF_PRINTF(fmt, ...) do {} while(0)
# define INF_STREAM(...)      do {} while(0)
#endif

#if (RABBITS_LOGLEVEL > 2)
# define DBG_PRINTF(fmt, ...) TRACE_PRINTF(LogLevel::DEBUG, fmt, ##__VA_ARGS__)
# define DBG_STREAM(...)      TRACE_STREAM(LogLevel::DEBUG, ##__VA_ARGS__)
#else
# define DBG_PRINTF(fmt, ...) do {} while(0)
# define DBG_STREAM(...)      do {} while(0)
#endif

#endif
