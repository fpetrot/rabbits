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
#include <functional>

#include <rabbits/logger/has_logger.h>
#include <rabbits/platform/description.h>
#include <rabbits/config/simu.h>

class JsonConsolePlugin;
class SignalEvent;

namespace protocol {

struct Version {
    static constexpr unsigned int PROTOCOL_VERSION = 1u;
};

enum Command {
    CMD_INVALID,
    CMD_PROTOCOL_VERSION,
    CMD_SIMU_STATUS,
    CMD_CONTINUE_ELABORATION,
    CMD_START_SIMULATION,
    CMD_PAUSE_SIMULATION,
    CMD_RESUME_SIMULATION,
    CMD_STOP_SIMULATION,
    CMD_ADD_BACKEND,
    CMD_ADD_GENERATOR,
    CMD_ADD_EVENT,
    CMD_MODIFY_GENERATOR,
    CMD_MODIFY_EVENT,
    CMD_GET_BACKEND_STATUS,
    CMD_GET_GENERATOR_STATUS,
    CMD_GET_EVENT_STATUS,
    CMD_DELETE_EVENT,
    CMD_FAILURE_REASON,
    CMD_READ_BACKEND,
    CMD_TRIGGER,
    CMD_SIMULATION_STARTED,
    CMD_SIMULATION_PAUSED,
    CMD_SIMULATION_RESUMED,
    CMD_SIMULATION_STOPPED,
};

enum Status {
    STA_BAD_CMD,
    STA_OK,
    STA_FAILURE,
    STA_EVENT,
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
    bool m_alive;
    std::vector<char> m_buffer;

    std::string m_client_pretty_addr;

    protocol::Command parse_command(PlatformDescription &d) const;
    void handle_cmd(PlatformDescription &d);

    template <protocol::Status s, protocol::Command c, class ...Args>
    void send_response(const Args&... args);

    bool check_signal_element(PlatformDescription &d);

    void continue_elaboration();
    void start_simulation();
    void resume_simulation();
    void pause_simulation();
    void stop_simulation();

    void add_backend(PlatformDescription &d);
    void add_generator(PlatformDescription &d);
    void add_event(PlatformDescription &d);
    void modify_generator(PlatformDescription &d);
    void modify_event(PlatformDescription &d);
    void get_backend_status(PlatformDescription &d);
    void get_generator_status(PlatformDescription &d);
    void get_event_status(PlatformDescription &d);
    void delete_event(PlatformDescription &d);
    void read_backend(PlatformDescription &d);

public:
    JsonConsoleClient(JsonConsolePlugin &parent,
                      boost::asio::io_service &io_service);
    virtual ~JsonConsoleClient();

    boost::asio::ip::tcp::socket& get_socket() { return m_socket; }
    void set_connected();
    bool is_alive() const { return m_alive; }

    void wait_for_cmd();
    void handle_cmd(const boost::system::error_code &, size_t);

    void signal_event(SignalEvent &ev);
    void simu_event(SimuEvent);

    const std::string & get_pretty_addr() const { return m_client_pretty_addr; }

    Logger & get_logger(LogContext::value context) const;
};

static inline std::ostream & operator<< (std::ostream &os, const JsonConsoleClient &c)
{
    os << c.get_pretty_addr();
    return os;
}
