/*
 *  This file is part of Rabbits
 *  Copyright (C) 2017  Clement Deschamps and Luc Michel
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

#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <map>

#include <rabbits/logger/has_logger.h>
#include <rabbits/platform/description.h>

class JsonConsolePlugin;

namespace protocol {

struct Version {
    static constexpr unsigned int PROTOCOL_VERSION = 1u;
};

enum Command {
    CMD_INVALID,
    CMD_PROTOCOL_VERSION,
    CMD_SIMU_STATUS,
    CMD_CONTINUE_ELABORATION,
    CMD_ADD_GENERATOR,
    CMD_FAILURE_REASON,
};

enum Status {
    STA_BAD_CMD,
    STA_OK,
    STA_FAILURE,
};

template <Status s>
struct StatusBuilder;

template <Command c, class ...Args>
struct CommandBuilder;

template <Status s, Command c, class ...Args>
class ResponseBuilder {
public:
    static std::string build(const Args&... args) {
        PlatformDescription d;

        StatusBuilder<s>::build(d);
        CommandBuilder<c, Args...>::build(d, args...);

        return d.dump_json();
    }
};

}

class JsonConsoleClient
    : public std::enable_shared_from_this<JsonConsoleClient>
    , public HasLoggerIface
{
public:
    typedef std::shared_ptr<JsonConsoleClient> Ptr;

    typedef const char * const CommandStr;

    struct CharStarCmp {
        bool operator() (CommandStr a, CommandStr b) const {
            return std::strcmp(a, b) < 0;
        }
    };

    typedef std::map<CommandStr, protocol::Command, CharStarCmp> CommandStrContainer;

    static const CommandStrContainer COMMAND_STR;

private:
    JsonConsolePlugin &m_parent;
    boost::asio::ip::tcp::socket m_socket;
    std::vector<char> m_buffer;

    protocol::Command parse_command(PlatformDescription &d) const;
    void handle_cmd(PlatformDescription &d);

    template <protocol::Status s, protocol::Command c, class ...Args>
    void send_response(const Args&... args);

    void add_generator(PlatformDescription &d);

public:
    JsonConsoleClient(JsonConsolePlugin &parent,
                      boost::asio::io_service &io_service);
    virtual ~JsonConsoleClient();

    boost::asio::ip::tcp::socket& get_socket() { return m_socket; }

    void wait_for_cmd();
    void handle_cmd(const boost::system::error_code &, size_t);

    std::string get_id() const;

    Logger & get_logger(LogContext::value context) const;
};
