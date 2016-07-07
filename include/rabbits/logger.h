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
#include <type_traits>

#include "rabbits/config.h"
#include "logger/logger.h"



#ifndef RABBITS_LOGLEVEL
# define RABBITS_LOGLEVEL 0
#endif

#define LOG_CHECK_ERR(logger) logger.next_trace(LogLevel::ERROR)


#if (RABBITS_LOGLEVEL > 0)
# define LOG_CHECK_WRN(logger) logger.next_trace(LogLevel::WARNING)
#else
# define LOG_CHECK_WRN(logger) false
#endif

#if (RABBITS_LOGLEVEL > 1)
# define LOG_CHECK_INF(logger) logger.next_trace(LogLevel::INFO)
#else
# define LOG_CHECK_INF(logger) false
#endif

#if (RABBITS_LOGLEVEL > 2)
# define LOG_CHECK_DBG(logger) logger.next_trace(LogLevel::DEBUG)
#else
# define LOG_CHECK_DBG(logger) false
#endif

#define LOG_CHECK(logger, lvl) LOG_CHECK_ ## lvl (logger)

#define LOG(ctx, lvl) \
    LOG_CHECK(Logger::get_root_logger(LogContext::ctx), lvl) && Logger::get_root_logger(LogContext::ctx)

#define MLOG(ctx, lvl) \
    LOG_CHECK(this->get_logger(LogContext::ctx), lvl) && this->get_logger(LogContext::ctx)

#define LOG_F(ctx, lvl, ...) \
    LOG(ctx, lvl) << Logger::format(__VA_ARGS__)

#define MLOG_F(ctx, lvl, ...) \
    MLOG(ctx, lvl) << Logger::format(__VA_ARGS__)

static inline Logger & get_logger(LogContext::value ctx) { return Logger::get_root_logger(ctx); }
static inline Logger & get_app_logger() { return Logger::get_root_logger(LogContext::APP); }
static inline Logger & get_sim_logger() { return Logger::get_root_logger(LogContext::SIM); }

#endif
