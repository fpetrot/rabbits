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

#include <boost/bind.hpp>

#include <rabbits/logger.h>

#include "client.h"
#include "json_console.h"

using std::string;

namespace protocol {

template <>
struct StatusBuilder<STA_OK> {
    static void build(PlatformDescription &d) {
        PlatformDescription dd;
        dd.load_json("{\"status\": \"ok\"}");
        d = d.merge(dd);
    }
};

template <>
struct StatusBuilder<STA_BAD_CMD> {
    static void build(PlatformDescription &d) {
        PlatformDescription dd;
        dd.load_json("{\"status\": \"bad_cmd\"}");
        d = d.merge(dd);
    }
};

template <>
struct StatusBuilder<STA_FAILURE> {
    static void build(PlatformDescription &d) {
        PlatformDescription dd;
        dd.load_json("{\"status\": \"failure\"}");
        d = d.merge(dd);
    }
};

template <>
struct CommandBuilder<CMD_INVALID> {
    static void build(PlatformDescription &d) {}
};

template <>
struct CommandBuilder<CMD_PROTOCOL_VERSION> {
    static void build(PlatformDescription &d) {
        PlatformDescription dd;
        std::stringstream ss;

        ss << "{\"version\": " << Version::PROTOCOL_VERSION << "}";
        dd.load_json(ss.str());
        d = d.merge(dd);
    }
};

template <>
struct CommandBuilder<CMD_FAILURE_REASON, const char *> {
    static void build(PlatformDescription &d, const char * reason) {
        PlatformDescription dd;
        std::string s;

        s = "{\"reason\": \"";
        s += reason;
        s += "\"}";

        dd.load_json(s);
        d = d.merge(dd);
    }
};

template <>
struct CommandBuilder<CMD_ADD_GENERATOR, string> {
    static void build(PlatformDescription &d, const string &name) {
        PlatformDescription dd;
        std::string s;

        s = "{\"generator\": \"";
        s += name;
        s += "\"}";

        dd.load_json(s);
        d = d.merge(dd);
    }
};

template <>
struct CommandBuilder<CMD_SIMU_STATUS, JsonConsolePlugin::SimulationStatus> {
    static constexpr const char * const STATUS_STR[] {
        [JsonConsolePlugin::BEFORE_ELABORATION] = "before_elaboration",
        [JsonConsolePlugin::BEFORE_SIMULATION]  = "before_simulation",
        [JsonConsolePlugin::SIMULATION_RUNNING] = "simulation_running",
        [JsonConsolePlugin::SIMULATION_PAUSED]  = "simulation_paused",
        [JsonConsolePlugin::SIMULATION_STOPPED] = "simulation_stopped",
        [JsonConsolePlugin::UNKNOWN]            = "unknown"
    };

    static void build(PlatformDescription &d,
                      const JsonConsolePlugin::SimulationStatus &sta) {
        PlatformDescription dd;
        std::stringstream ss;

        ss << "{\"simulation_status\": \"" << STATUS_STR[sta] << "\"}";
        dd.load_json(ss.str());
        d = d.merge(dd);
    }
};

template <>
struct CommandBuilder<CMD_CONTINUE_ELABORATION> {
    static void build(PlatformDescription &d) {}
};

constexpr const char * const
CommandBuilder<CMD_SIMU_STATUS, JsonConsolePlugin::SimulationStatus>::STATUS_STR[];

}

const JsonConsoleClient::CommandStrContainer JsonConsoleClient::COMMAND_STR {
    { "get_version", protocol::CMD_PROTOCOL_VERSION },
    { "get_status", protocol::CMD_SIMU_STATUS },
    { "continue_elaboration", protocol::CMD_CONTINUE_ELABORATION },
    { "add_generator", protocol::CMD_ADD_GENERATOR },
};


JsonConsoleClient::JsonConsoleClient(JsonConsolePlugin &parent,
                                     boost::asio::io_service &io_service)
    : m_parent(parent)
    , m_socket(io_service)
{
    m_buffer.resize(1024);
}

JsonConsoleClient::~JsonConsoleClient()
{
    boost::system::error_code ec;

    MLOG(APP, TRC) << "Destructing client " << get_id() << "\n";
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

    if (ec) {
        MLOG(APP, DBG) << "Error while calling shutdown: " << ec << "\n";
    }

    m_socket.close();
}

void JsonConsoleClient::wait_for_cmd()
{
    m_socket.async_read_some(boost::asio::buffer(m_buffer),
                             boost::bind(&JsonConsoleClient::handle_cmd, shared_from_this(),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

template <protocol::Status s, protocol::Command c, class ...Args>
void JsonConsoleClient::send_response(const Args&... args)
{
    std::string response;

    response = protocol::ResponseBuilder<s, c, Args...>::build(args...);
    m_socket.write_some(boost::asio::buffer(response));
}

protocol::Command JsonConsoleClient::parse_command(PlatformDescription &d) const
{
    if (!d.exists("cmd")) {
        return protocol::CMD_INVALID;
    }

    const string cmd = d["cmd"].as<string>();

    if (COMMAND_STR.find(cmd.c_str()) == COMMAND_STR.end()) {
        return protocol::CMD_INVALID;
    }

    return COMMAND_STR.at(cmd.c_str());
}

void JsonConsoleClient::add_generator(PlatformDescription &d)
{
    using namespace protocol;

    if (m_parent.get_simulation_status() != JsonConsolePlugin::BEFORE_ELABORATION) {
        const char * msg = "cannot create generator after elaboration";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    if (!d.exists("type")) {
        const char * msg = "missing stub type";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    if (!d.exists("component")) {
        const char * msg = "missing target component";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    if (!d.exists("port")) {
        const char * msg = "missing target port";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    SignalGenerator &gen = m_parent.create_generator(d);

    send_response<STA_OK, CMD_ADD_GENERATOR>(gen.name);
}

void JsonConsoleClient::handle_cmd(PlatformDescription &d)
{
    using namespace protocol;

    Command cmd = parse_command(d);

    switch (cmd) {
    case CMD_INVALID:
    case CMD_FAILURE_REASON:
        send_response<STA_BAD_CMD, CMD_INVALID>();
        break;
    case CMD_PROTOCOL_VERSION:
        send_response<STA_OK, CMD_PROTOCOL_VERSION>();
        break;
    case CMD_SIMU_STATUS:
        send_response<STA_OK, CMD_SIMU_STATUS>(m_parent.get_simulation_status());
        break;
    case CMD_CONTINUE_ELABORATION:
        if (m_parent.get_simulation_status() == JsonConsolePlugin::BEFORE_ELABORATION) {
            m_parent.continue_elaboration();
            send_response<STA_OK, CMD_CONTINUE_ELABORATION>();
        } else {
            const char * msg = "elaboration already done";
            send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        }
        break;
    case CMD_ADD_GENERATOR:
        add_generator(d);
        break;
    }
}

void JsonConsoleClient::handle_cmd(const boost::system::error_code &err, size_t size)
{
    if (!err) {
        std::string str(&(m_buffer[0]), size);
        MLOG(APP, TRC) << "Got a cmd size:" << size << "\n" << str << "\n";

        PlatformDescription d;

        try {
            d.load_json(str);
            handle_cmd(d);
            wait_for_cmd();
        } catch (PlatformDescription::JsonParsingException e) {
            MLOG(APP, DBG) << "JSON parsing error: " << e.what() << "\n";
        }

    } else if (err == boost::asio::error::eof) {
        MLOG(APP, TRC) << "Client " << get_id() << " closed connection\n";
    } else {
        MLOG(APP, DBG) << "Client " << get_id() << ": error while waiting for command\n";
    }
}


std::string JsonConsoleClient::get_id() const
{
    if (m_socket.is_open()) {
        std::stringstream ss;

        ss << m_socket.remote_endpoint().address().to_string()
           << ":"
           << m_socket.remote_endpoint().port();

        return ss.str();
    } else {
        return "(not connected)";
    }
}

Logger & JsonConsoleClient::get_logger(LogContext::value context) const
{
    return m_parent.get_logger(context);
}
