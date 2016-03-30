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

/**
 * @file logger.h
 * @brief Logger class declaration
 */

#ifndef _RABBITS_LOGGER_LOGGER_H
#define _RABBITS_LOGGER_LOGGER_H

#include "has_logger_iface.h"

/**
 * @brief Main logging system.
 */
class Logger : public HasLoggerIface {
public:
    static const std::string PREFIXES[];

protected:
    typedef std::vector<char>::size_type size_type;
    static const size_type DEFAULT_BUF_SIZE = 256;

private:
    static Logger m_logger;

    Logger() : m_level(LogLevel::value(RABBITS_LOGLEVEL)), m_banner_enabled(true), m_muted(false)  {
        for (int i = 0; i < LogLevel::LASTLOGLVL; i++) {
            m_streams[i] = &std::cerr;
        }

        m_format_buf.resize(DEFAULT_BUF_SIZE);
    }

    Logger(const Logger&);
    Logger& operator=(const Logger&);

protected:
    LogLevel::value m_level;
    bool m_banner_enabled;
    bool m_muted;

    std::ostream * m_streams[LogLevel::LASTLOGLVL];
    mutable std::ofstream m_null_stream;

    typedef std::stack<std::vector<std::ios::fmtflags> > StateStack;
    StateStack m_state_stack;

    mutable std::vector<char> m_format_buf;

public:
    /**
     * @brief Return the singleton instance of the Logger.
     *
     * @return the singleton instance of the Logger.
     */
    static Logger & get() { return m_logger; }

    /**
     * @brief Set the stream associated with the given log level.
     *
     * Default stream is std::cout for all log levels.
     *
     * @param[in] lvl The log level.
     * @param[in] str The stream to associate to the log level.
     */
    void set_stream(LogLevel::value lvl, std::ostream *str) { m_streams[lvl] = str; }

    /**
     * @brief Set the current maximum log level displayed.
     *
     * All traces having a log level greater than the lvl value will be
     * discarded and won't be given to their corresponding stream.
     *
     * @param[in] lvl The log level to set.
     */
    void set_log_level(LogLevel::value lvl) { m_level = lvl; }

    /**
     * @brief Get the current log level.
     *
     * @return the current log level.
     */
    LogLevel::value get_log_level() { return m_level; }

    int log_printf(LogLevel::value lvl, const std::string fmt, ...) const ;
    int log_vprintf(LogLevel::value lvl, const std::string fmt, va_list ap) const;
    std::ostream & log_stream(LogLevel::value lvl) const;

    /**
     * @brief Save the current streams flags.
     */
    void save_flags();

    /**
     * @brief Restore the streams flags previously saved.
     */
    void restore_flags();

    /**
     * @brief Enable or disable the banner emitted by the Logger such as "[warn]".
     *
     * @param enabled the desired banner state (enabled or disabled).
     *
     * @return The previous state.
     */
    bool enable_banner(bool enabled) {
        bool save = m_banner_enabled;
        m_banner_enabled = enabled;
        return save;
    }

    /**
     * @brief Mute the Logger.
     *
     * All traces will be discarded. Nothing will be logged.
     */
    void mute() { m_muted = true; }

    /**
     * @brief Unmute the Logger.
     */
    void unmute() { m_muted = false; }
};
#endif
