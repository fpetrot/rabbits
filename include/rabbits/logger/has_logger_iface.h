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

#ifndef _RABBITS_LOGGER_HAS_LOGGER_IFACE_H
#define _RABBITS_LOGGER_HAS_LOGGER_IFACE_H

#include <cstdarg>

/**
 * @brief Log level.
 */
class LogLevel {
public:
    enum value {
        ERROR = 0, WARNING, INFO, DEBUG,
        LASTLOGLVL
    };
};

/**
 * @brief Logger interface.
 *
 * Classes willing to override log traces must implement this interface.
 */
class HasLoggerIface {
    /**
     * @brief Return an std::ostream object associated to the log level lvl.
     *
     * @param[in] lvl The wanted log level.
     *
     * @return an std::ostream object associated to the log level lvl.
     */
    virtual std::ostream & log_stream(LogLevel::value lvl) const = 0;

    /**
     * @brief vprintf-like function for logging.
     *
     * @param[in] lvl The wanted log level.
     * @param[in] fmt The printf-like format string.
     * @param[in,out] ap The va_list corresponding to fmt.
     *
     * @return The number of character printed.
     */
    virtual int log_vprintf(LogLevel::value lvl, const std::string fmt, va_list ap) const = 0;

    /**
     * @brief printf-like function for logging
     *
     * @param[in] lvl The wanted log level.
     * @param[in] fmt The printf-like format string.
     * @param[in,out] ... The variable number of parameters corresponding to fmt.
     *
     * @return The number of character printed.
     */
    virtual int log_printf(LogLevel::value lvl, const std::string fmt, ...) const = 0;
};

#endif
