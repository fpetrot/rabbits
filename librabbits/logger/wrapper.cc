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

#include "rabbits/logger/wrapper.h"
#include "rabbits/logger.h"
#include "rabbits/module/parameters.h"


LoggerWrapper::LoggerWrapper(const std::string & name, HasLoggerIface &parent, Parameters &params, ConfigManager &config)
    : m_logger_app(config)
    , m_logger_sim(config)
    , m_name(name)
    , m_params(params)
{
    if (m_params.exists("log-file")) {
        m_params["log-file"].set_default(name + ".log");
    }

    setup_loggers(&parent);
}

LoggerWrapper::LoggerWrapper(Parameters &params, ConfigManager &config)
    : m_logger_app(config)
    , m_logger_sim(config)
    , m_params(params)
{
    setup_loggers(nullptr);
}

LoggerWrapper::LogTarget LoggerWrapper::get_log_target(const std::string target_s)
{
    if (target_s == "stdout") {
        return LT_STDOUT;
    } else if (target_s == "stderr") {
        return LT_STDERR;
    } else if (target_s == "file") {
        return LT_FILE;
    }

    LOG(APP, ERR) << "Ignoring invalid log target " << target_s << "\n";
    return LT_STDERR;
}

LogLevel::value LoggerWrapper::get_log_level(const std::string level_s)
{
    if (level_s == "debug") {
        return LogLevel::DEBUG;
    } else if (level_s == "info") {
        return LogLevel::INFO;
    } else if (level_s == "warning") {
        return LogLevel::WARNING;
    } else if (level_s == "error") {
        return LogLevel::ERROR;
    } else if (level_s == "trace") {
        return LogLevel::TRACE;
    }

    LOG(APP, ERR) << "Ignoring invalid log level " << level_s << "\n";
    return LogLevel::INFO;
}

std::fstream* LoggerWrapper::open_file(const std::string &fn)
{
    std::fstream* ret = nullptr;

    if (m_log_files.find(fn) != m_log_files.end()) {
        if (!m_log_files[fn]) {
            return nullptr;
        }
        return m_log_files[fn].get();
    }

    ret = new std::fstream(fn, std::fstream::out | std::fstream::trunc);

    m_log_files[fn].reset(ret);

    return ret;
}

void LoggerWrapper::setup_logger_banner(Logger &l)
{
    if (m_name.empty()) {
        return;
    }

    std::stringstream banner;
    banner << "[" << m_name << "]";
    l.set_custom_banner(banner.str());

    l.set_custom_banner([] (Logger &l, const std::string &banner)
                        {
                        l << format::cyan << banner << format::reset;
                        });
}

void LoggerWrapper::setup_logger(Logger &l, LogTarget target, LogLevel::value lvl,
                  const std::string log_file)
{
    setup_logger_banner(l);

    switch (target) {
    case LT_STDOUT:
        l.set_streams(&std::cout);
        break;

    case LT_FILE:
        {
            std::fstream* file = open_file(log_file);

            if (!*file) {
                LOG(APP, ERR) << "Unable to open log file "
                    << log_file << ". Falling back to stderr\n";
            } else {
                l.set_streams(file);
            }
        }
        break;

    case LT_STDERR:
        /* Default */
        break;
    }

    l.set_log_level(lvl);
}

bool LoggerWrapper::logger_is_custom()
{
    if (!m_params.exists("log-file")) {
        return false;
    }

    return (!m_params["log-target"].is_default())
        || (!m_params["log-level"].is_default())
        || (!m_params["log-file"].is_default())
        || (!m_params["debug"].is_default())
        || (!m_params["trace"].is_default());
}

void LoggerWrapper::setup_loggers(HasLoggerIface *parent)
{
    LogTarget log_target;
    LogLevel::value log_level;
    std::string log_file;
    bool debug = false;
    bool trace = false;
    bool custom = logger_is_custom();

    if (custom) {
        log_target = get_log_target(m_params["log-target"].as<std::string>());
        log_level = get_log_level(m_params["log-level"].as<std::string>());
        log_file = m_params["log-file"].as<std::string>();
        debug = m_params["debug"].as<bool>();
        trace = m_params["trace"].as<bool>();
    } else {
        log_target = LT_STDERR;
        log_level = LogLevel::INFO;
        log_file = "";
    }

    if (debug) {
        log_level = LogLevel::DEBUG;
    }

    if (trace) {
        log_level = LogLevel::TRACE;
    }

    for (int i = 0; i < LogContext::LASTLOGCONTEXT; i++) {
        parent->get_logger(LogContext::value(i)).set_child(*m_loggers[i]);

        if (custom) {
            setup_logger(*m_loggers[i], log_target, log_level, log_file);
        } else {
            setup_logger_banner(*m_loggers[i]);
        }
    }
}
