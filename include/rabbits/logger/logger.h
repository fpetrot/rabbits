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

#include <iostream>
#include <vector>
#include <stack>
#include <ostream>
#include <functional>

#include "datatypes.h"
#include "rabbits/config.h"
#include "format.h"

class ConfigManager;

/**
 * @brief Main logging system.
 */
class Logger {
public:
    struct Stream {
        std::ostream *sink = nullptr;
        Formatter *formatter = nullptr;

        Stream() {}
        Stream(std::ostream &sink, ConfigManager &config) : sink(&sink), formatter(new Formatter(sink, config)) {}

        Stream(const Stream &s) : sink(s.sink), formatter(new Formatter(*s.formatter)) {}
        Stream(Stream &&s) : sink(s.sink), formatter(s.formatter) { s.sink = nullptr; s.formatter = nullptr; }

        ~Stream() { delete formatter; }

        Stream & operator= (Stream s) { std::swap(sink, s.sink); std::swap(formatter, s.formatter); return *this; }

        bool is_null() const { return sink == nullptr; }
    };

    typedef Logger & (*formater_fn)(Logger &);

    static const std::string PREFIXES[];
    static const formater_fn PREFIX_COLORS[];

protected:
    typedef std::vector<char>::size_type size_type;
    static const size_type DEFAULT_BUF_SIZE = 256;

protected:
    static std::vector<char> m_format_buf;
    static char * vformat(const char *fmt, va_list ap);

    Logger *m_parent = nullptr;
    LogLevel::value m_level = LogLevel::value(RABBITS_LOGLEVEL);
    LogLevel::value m_next_lvl;

    ConfigManager &m_config;

    bool m_new_trace = true;

    bool m_banner_enabled = true;
    std::string m_custom_banner;
    std::function<void(Logger&, const std::string&)> m_banner_cb;

    bool m_muted = false;
    bool m_auto_reset = true;

    Stream m_streams[LogLevel::LASTLOGLVL];

    typedef std::stack< std::vector<std::ios::fmtflags> > StateStack;
    StateStack m_state_stack;


    void clear_streams()
    {
        for (int i = 0; i < LogLevel::LASTLOGLVL; i++) {
            m_streams[i] = Stream();
        }
    }

    Stream & get_stream(LogLevel::value lvl)
    {
        if (m_streams[lvl].is_null() && m_parent) {
            return m_parent->get_stream(lvl);
        } else {
            return m_streams[lvl];
        }
    }

    const Stream & get_stream(LogLevel::value lvl) const
    {
        if (m_streams[lvl].is_null() && m_parent) {
            return m_parent->get_stream(lvl);
        } else {
            return m_streams[lvl];
        }
    }

    std::ostream& get_sink(LogLevel::value lvl)
    {
        return *(get_stream(lvl).sink);
    }

    void _emit_banner(Logger &l, LogLevel::value lvl, std::ostream &s) {
        if (m_parent) {
            m_parent->_emit_banner(l, lvl, s);
        } else {
            PREFIX_COLORS[lvl](l);
            s << PREFIXES[lvl];
            l.reset_format();
        }

        if (m_banner_cb) {
            m_banner_cb(l, m_custom_banner);
        } else {
            s << m_custom_banner;
        }
    }

    void emit_banner(std::ostream &s)
    {
        if (m_new_trace && m_banner_enabled) {
            m_new_trace = false;

            _emit_banner(*this, m_next_lvl, s);

            s << " ";
        }
    }

public:
    Logger(std::ostream *default_stream, ConfigManager &config)
        : m_config(config)
    {
        for (int i = 0; i < LogLevel::LASTLOGLVL; i++) {
            m_streams[i] = Stream(*default_stream, m_config);
        }
    }

    explicit Logger(ConfigManager &config)
        : m_config(config)
    {
        for (int i = 0; i < LogLevel::LASTLOGLVL; i++) {
            m_streams[i] = Stream(std::cerr, m_config);
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static char * format(const char *fmt, ...);

    /**
     * @brief Set the stream associated with the given log level.
     *
     * Default stream is std::cerr for all log levels.
     *
     * @param[in] lvl The log level.
     * @param[in] str The stream to associate to the log level.
     */
    void set_stream(LogLevel::value lvl, std::ostream *str) { m_streams[lvl] = Stream(*str, m_config); }

    /**
     * @brief Set the stream for all log levels.
     *
     * Default stream is std::cerr.
     *
     * @param[in] str The stream to set for all log levels.
     */
    void set_streams(std::ostream *str)
    {
        for (int i = 0; i < int(LogLevel::LASTLOGLVL); i++) {
            m_streams[i] = Stream(*str, m_config);
        }
    }

    /**
     * @brief Set the current maximum log level displayed.
     *
     * All traces having a log level greater than the lvl value will be
     * discarded and won't be given to their corresponding stream.
     *
     * @param[in] lvl The log level to set.
     */
    void set_log_level(LogLevel::value lvl) { m_level = lvl; }

    void set_color(ConsoleColor::value c, ConsoleAttr::value a)
    {
        Stream &s = get_stream(m_next_lvl);

        if (s.is_null()) {
            return;
        }

        s.formatter->set_color(c, a);
    }

    void reset_format()
    {
        Stream &s = get_stream(m_next_lvl);

        if (s.is_null()) {
            return;
        }

        s.formatter->reset();
    }

    /**
     * @brief Get the current log level.
     *
     * @return the current log level.
     */
    LogLevel::value get_log_level() { return m_level; }

    /* XXX deprecated */
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
     * @param[in] enabled the desired banner state (enabled or disabled).
     *
     * @return The previous state.
     */
    bool enable_banner(bool enabled) {
        bool save = m_banner_enabled;
        m_banner_enabled = enabled;
        return save;
    }

    bool is_tty(LogLevel::value lvl) const {
        return get_stream(lvl).formatter->is_tty();
    }

    void get_tty_attr(LogLevel::value lvl, int &rows, int &cols) const {
        return get_stream(lvl).formatter->get_tty_attr(rows, cols);
    }

    /**
     * @brief Enable or disable automatic reset of the logger format at the end of a trace.
     *
     * When enabled, a format reset is performed at the end of each trace so
     * that any modification (such as text color) does not impact the next
     * traces.
     *
     * @param[in] enabled Enable or disable auto reset.
     *
     * @return The previous state.
     */
    bool enable_auto_reset(bool enabled) {
        bool save = m_auto_reset;
        m_auto_reset = enabled;
        return save;
    }

    /**
     * @brief Set a custom banner to append to the log level prefix
     *
     * @param[in] banner The banner
     */
    void set_custom_banner(const std::string &banner) {
        m_custom_banner = banner;
    }

    /**
     * @brief Set a callback used to emit the banner for each trace
     *
     * @param[in] f The banner emitter that takes as parameter the current
     *              logger and the text banner.
     */
    void set_custom_banner(const std::function<void(Logger&, const std::string&)> &f)
    {
        m_banner_cb = f;
    }

    /**
     * @brief Append a string to the custom banner
     *
     * @param[in] suffix The string to append
     */
    void append_to_custom_banner(const std::string &suffix) {
        m_custom_banner += suffix;
    }

    /**
     * @brief Remove any previously set custom banner
     */
    void clear_custom_banner() {
        m_custom_banner = "";
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

    /**
     * @brief Set the logger instance given as parameter to be a child of this
     * logger instance.
     *
     * A child logger uses the same output streams as the parent and has the
     * same log level, until set otherwise.
     *
     * @param[in,out] l The instance to set as a child of this instance.
     */
    void set_child(Logger &l)
    {
        l.m_level = m_level;
        l.m_banner_enabled = m_banner_enabled;
        l.m_custom_banner = "";
        l.m_muted = m_muted;

        l.clear_streams();

        l.m_parent = this;
    }

    /**
     * @brief Set the log level of the next trace.
     *
     * This method must be called before each trace. It returns false if logs
     * are disabled for the given level. In this case, a new call
     * to <code>next_trace</code> must be performed before attempting to trace something.
     *
     * This method is not supposed to be used alone and is called by the LOG() macros family.
     *
     * @param[in] lvl The desired level of the next trace
     *
     * @return true if the logger is enabled for the given level lvl, false otherwise.
     */
    bool next_trace(LogLevel::value lvl)
    {
        if ((lvl > m_level) || m_muted) {
            return false;
        }

        m_next_lvl = lvl;
        m_new_trace = true;

        if (m_auto_reset) {
            reset_format();
        }

        return true;
    }

    template <class T>
    Logger & operator << (const T& t)
    {
        std::ostream &s = get_sink(m_next_lvl);
        emit_banner(s);
        s << t;
        return *this;
    }

    Logger & operator << (std::ostream& (*pf)(std::ostream&))
    {
        std::ostream &s = get_sink(m_next_lvl);
        emit_banner(s);
        s << *pf;
        return *this;
    }

    Logger & operator << (std::ios& (*pf)(std::ios&))
    {
        std::ostream &s = get_sink(m_next_lvl);
        emit_banner(s);
        s << *pf;
        return *this;
    }

    Logger & operator << (std::ios_base& (*pf)(std::ios_base&))
    {
        std::ostream &s = get_sink(m_next_lvl);
        emit_banner(s);
        s << *pf;
        return *this;
    }

    Logger & operator << (Logger & (*pf)(Logger &))
    {
        std::ostream &s = get_sink(m_next_lvl);
        emit_banner(s);
        return (*pf)(*this);
    }

    explicit operator bool() const { return true; }
};

namespace format {
    static inline Logger & reset(Logger &l) { l.reset_format(); return l; }

    static inline Logger & black(Logger &l) { l.set_color(ConsoleColor::BLACK, ConsoleAttr::NORMAL); return l; }
    static inline Logger & blue(Logger &l) { l.set_color(ConsoleColor::BLUE, ConsoleAttr::NORMAL); return l; }
    static inline Logger & green(Logger &l) { l.set_color(ConsoleColor::GREEN, ConsoleAttr::NORMAL); return l; }
    static inline Logger & cyan(Logger &l) { l.set_color(ConsoleColor::CYAN, ConsoleAttr::NORMAL); return l; }
    static inline Logger & red(Logger &l) { l.set_color(ConsoleColor::RED, ConsoleAttr::NORMAL); return l; }
    static inline Logger & purple(Logger &l) { l.set_color(ConsoleColor::PURPLE, ConsoleAttr::NORMAL); return l; }
    static inline Logger & yellow(Logger &l) { l.set_color(ConsoleColor::YELLOW, ConsoleAttr::NORMAL); return l; }
    static inline Logger & white(Logger &l) { l.set_color(ConsoleColor::WHITE, ConsoleAttr::NORMAL); return l; }

    static inline Logger & black_b(Logger &l) { l.set_color(ConsoleColor::BLACK, ConsoleAttr::BOLD); return l; }
    static inline Logger & blue_b(Logger &l) { l.set_color(ConsoleColor::BLUE, ConsoleAttr::BOLD); return l; }
    static inline Logger & green_b(Logger &l) { l.set_color(ConsoleColor::GREEN, ConsoleAttr::BOLD); return l; }
    static inline Logger & cyan_b(Logger &l) { l.set_color(ConsoleColor::CYAN, ConsoleAttr::BOLD); return l; }
    static inline Logger & red_b(Logger &l) { l.set_color(ConsoleColor::RED, ConsoleAttr::BOLD); return l; }
    static inline Logger & purple_b(Logger &l) { l.set_color(ConsoleColor::PURPLE, ConsoleAttr::BOLD); return l; }
    static inline Logger & yellow_b(Logger &l) { l.set_color(ConsoleColor::YELLOW, ConsoleAttr::BOLD); return l; }
    static inline Logger & white_b(Logger &l) { l.set_color(ConsoleColor::WHITE, ConsoleAttr::BOLD); return l; }
}

#endif
