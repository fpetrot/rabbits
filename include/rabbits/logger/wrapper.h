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

#ifndef _RABBITS_LOGGER_WRAPPER_H
#define _RABBITS_LOGGER_WRAPPER_H

#include <map>
#include <memory>
#include <fstream>

#include "logger.h"
#include "has_logger.h"

class Parameters;

class LoggerWrapper : public HasLoggerIface {
public:
    enum LogTarget {
        LT_STDOUT, LT_STDERR, LT_FILE
    };

    typedef std::unique_ptr<std::fstream> LogFile;
    typedef std::map<std::string, LogFile> LogFiles;

protected:
    mutable Logger m_logger_app;
    mutable Logger m_logger_sim;

    std::string m_name;
    Parameters &m_params;

    Logger * m_loggers[LogContext::LASTLOGCONTEXT] { &m_logger_app, &m_logger_sim };

    LogFiles m_log_files;

    LogTarget get_log_target(const std::string target_s);
    LogLevel::value get_log_level(const std::string level_s);
    std::fstream* open_file(const std::string &fn);
    void setup_logger_banner(Logger &l);
    void setup_logger(Logger &l, LogTarget target, LogLevel::value lvl,
                      const std::string log_file);
    bool logger_is_custom();
    void setup_loggers(HasLoggerIface &parent);


public:
    LoggerWrapper(const std::string & name, HasLoggerIface &parent, Parameters &params, ConfigManager &config);
    LoggerWrapper(Parameters &params, ConfigManager &config);

    virtual ~LoggerWrapper() {}

    /* HasLoggerIface */
    Logger & get_logger(LogContext::value context) const { return *m_loggers[context]; }
};

#endif
