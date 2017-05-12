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

#include <rabbits/logger.h>

#include "client.h"
#include "json_console.h"
#include "backend.h"

/* Fix compilation with Clang and Boost versions ~<=1.55 */
#ifdef BOOST_NO_CXX11_SMART_PTR
namespace boost {
template<class T> const T* get_pointer(std::shared_ptr<T> const& p)
{
    return p.get();
}

template<class T> T* get_pointer(std::shared_ptr<T>& p)
{
    return p.get();
}
} // namespace boost
#endif

#include <boost/bind.hpp>

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
struct StatusBuilder<STA_EVENT> {
    static void build(PlatformDescription &d) {
        PlatformDescription dd;
        dd.load_json("{\"status\": \"event\"}");
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
struct CommandBuilder<CMD_ADD_EVENT, string> {
    static void build(PlatformDescription &d, const string &name) {
        PlatformDescription dd;
        std::string s;

        s = "{\"trigger\": \"";
        s += name;
        s += "\"}";

        dd.load_json(s);
        d = d.merge(dd);
    }
};

template <>
struct CommandBuilder<CMD_ADD_BACKEND, string> {
    static void build(PlatformDescription &d, const string &name) {
        PlatformDescription dd;
        std::string s;

        s = "{\"backend\": \"";
        s += name;
        s += "\"}";

        dd.load_json(s);
        d = d.merge(dd);
    }
};

template <>
struct CommandBuilder<CMD_READ_BACKEND, PlatformDescription> {
    static void build(PlatformDescription &d, PlatformDescription serialized) {
        d = d.merge(serialized);
    }
};

template <>
struct CommandBuilder<CMD_MODIFY_GENERATOR> {
    static void build(PlatformDescription &d) {}
};

template <>
struct CommandBuilder<CMD_MODIFY_EVENT> {
    static void build(PlatformDescription &d) {}
};

template <>
struct CommandBuilder<CMD_DELETE_EVENT> {
    static void build(PlatformDescription &d) {}
};

template <>
struct CommandBuilder<CMD_GET_BACKEND_STATUS,
                      SignalElement::Status,
                      SignalElement::FailureReason> {
    static constexpr const char * const STATUS_STR[] {
        [SignalElement::STA_NEW] = "new",
        [SignalElement::STA_CREATED] = "created",
        [SignalElement::STA_FAILURE] = "failure"
    };

    static constexpr const char * const REASON_STR[] {
        [SignalElement::FAIL_NO_FAILURE] = "no_failure",
        [SignalElement::FAIL_INVALID_TYPE] = "invalid_type",
        [SignalElement::FAIL_COMP_NOT_FOUND] = "component_not_found",
        [SignalElement::FAIL_PORT_NOT_FOUND] = "port_not_found",
        [SignalElement::FAIL_BINDING] = "binding_failure",
        [SignalElement::FAIL_INTERNAL] = "internal_error",
    };

    static void build(PlatformDescription &d,
                      SignalElement::Status sta, SignalElement::FailureReason r)
    {
        PlatformDescription dd;
        std::string s;

        s = "{\"backend_status\": \"";
        s += STATUS_STR[sta];
        s += "\"";

        if (sta == SignalElement::STA_FAILURE) {
            s += ", \"failure\": \"";
            s += REASON_STR[r];
            s += "\"";
        }

        s += "}";

        dd.load_json(s);
        d = d.merge(dd);
    }
};

template <>
struct CommandBuilder<CMD_GET_GENERATOR_STATUS,
                      SignalElement::Status,
                      SignalElement::FailureReason> {

    typedef CommandBuilder<CMD_GET_BACKEND_STATUS,
                           SignalElement::Status,
                           SignalElement::FailureReason> BackendBuilder;

    static void build(PlatformDescription &d,
                      SignalElement::Status sta, SignalElement::FailureReason r)
    {
        PlatformDescription dd;
        std::string s;

        s = "{\"generator_status\": \"";
        s += BackendBuilder::STATUS_STR[sta];
        s += "\"";

        if (sta == SignalElement::STA_FAILURE) {
            s += ", \"failure\": \"";
            s += BackendBuilder::REASON_STR[r];
            s += "\"";
        }

        s += "}";

        dd.load_json(s);
        d = d.merge(dd);
    }
};

template <>
struct CommandBuilder<CMD_GET_EVENT_STATUS,
                      SignalElement::Status,
                      SignalElement::FailureReason> {

    typedef CommandBuilder<CMD_GET_BACKEND_STATUS,
                           SignalElement::Status,
                           SignalElement::FailureReason> BackendBuilder;

    static void build(PlatformDescription &d,
                      SignalElement::Status sta, SignalElement::FailureReason r)
    {
        PlatformDescription dd;
        std::string s;

        s = "{\"trigger_status\": \"";
        s += BackendBuilder::STATUS_STR[sta];
        s += "\"";

        if (sta == SignalElement::STA_FAILURE) {
            s += ", \"failure\": \"";
            s += BackendBuilder::REASON_STR[r];
            s += "\"";
        }

        s += "}";

        dd.load_json(s);
        d = d.merge(dd);
    }
};

template <>
struct CommandBuilder<CMD_TRIGGER, string> {
    static void build(PlatformDescription &d, const string &name) {
        PlatformDescription dd;
        std::string s;

        s = "{\"event\": \"trigger\", \"trigger\": \"";
        s += name;
        s += "\"}";

        dd.load_json(s);
        d = d.merge(dd);
    }
};

template <>
struct CommandBuilder<CMD_SIMULATION_PAUSED> {
    static void build(PlatformDescription &d) {
        PlatformDescription dd;
        std::string s;

        s = "{\"event\": \"simulation_paused\"}";

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

template <>
struct CommandBuilder<CMD_START_SIMULATION> {
    static void build(PlatformDescription &d) {}
};

template <>
struct CommandBuilder<CMD_RESUME_SIMULATION> {
    static void build(PlatformDescription &d) {}
};

template <>
struct CommandBuilder<CMD_PAUSE_SIMULATION> {
    static void build(PlatformDescription &d) {}
};


constexpr const char * const
CommandBuilder<CMD_GET_BACKEND_STATUS,
               SignalElement::Status,
               SignalElement::FailureReason>::STATUS_STR[];

constexpr const char * const
CommandBuilder<CMD_GET_BACKEND_STATUS,
               SignalElement::Status,
               SignalElement::FailureReason>::REASON_STR[];

constexpr const char * const
CommandBuilder<CMD_SIMU_STATUS, JsonConsolePlugin::SimulationStatus>::STATUS_STR[];

} /* namespace protocol */


const JsonConsoleClient::CommandStrContainer JsonConsoleClient::COMMAND_STR {
    { "get_version", protocol::CMD_PROTOCOL_VERSION },
    { "get_status", protocol::CMD_SIMU_STATUS },
    { "continue_elaboration", protocol::CMD_CONTINUE_ELABORATION },
    { "start_simulation", protocol::CMD_START_SIMULATION },
    { "pause_simulation", protocol::CMD_PAUSE_SIMULATION },
    { "resume_simulation", protocol::CMD_RESUME_SIMULATION },
    { "add_backend", protocol::CMD_ADD_BACKEND },
    { "add_generator", protocol::CMD_ADD_GENERATOR },
    { "add_trigger", protocol::CMD_ADD_EVENT },
    { "modify_generator", protocol::CMD_MODIFY_GENERATOR },
    { "modify_trigger", protocol::CMD_MODIFY_EVENT },
    { "get_backend_status", protocol::CMD_GET_BACKEND_STATUS },
    { "get_generator_status", protocol::CMD_GET_GENERATOR_STATUS },
    { "get_trigger_status", protocol::CMD_GET_EVENT_STATUS },
    { "delete_trigger", protocol::CMD_DELETE_EVENT },
    { "read_backend", protocol::CMD_READ_BACKEND },
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

void JsonConsoleClient::add_backend(PlatformDescription &d)
{
    using namespace protocol;

    if (m_parent.get_simulation_status() != JsonConsolePlugin::BEFORE_ELABORATION) {
        const char * msg = "cannot create a backend after elaboration";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    if (!d.exists("type")) {
        const char * msg = "missing backend type";
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

    const BackendInstance &b =  m_parent.create_backend(d);

    send_response<STA_OK, CMD_ADD_BACKEND>(b.get_name());

}

bool JsonConsoleClient::check_signal_element(PlatformDescription &d)
{
    using namespace protocol;

    if (!d.exists("backend")) {
        const char * msg = "missing target backend";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return false;
    }

    if (!m_parent.backend_exists(d["backend"].as<string>())) {
        const char * msg = "unknown backend";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return false;
    }

    return true;
}

void JsonConsoleClient::add_generator(PlatformDescription &d)
{
    using namespace protocol;

    if (!check_signal_element(d)) {
        return;
    }

    SignalGenerator::Ptr elt = m_parent.create_generator(d);
    send_response<STA_OK, CMD_ADD_GENERATOR>(elt->get_name());
}

void JsonConsoleClient::add_event(PlatformDescription &d)
{
    using namespace protocol;

    if (!check_signal_element(d)) {
        return;
    }

    SignalEvent::Ptr elt = m_parent.create_event(d, shared_from_this());
    send_response<STA_OK, CMD_ADD_EVENT>(elt->get_name());
}

void JsonConsoleClient::read_backend(PlatformDescription &d)
{
    using namespace protocol;

    if (m_parent.get_simulation_status() <= JsonConsolePlugin::BEFORE_SIMULATION) {
        const char * msg = "cannot read a backend while simulation isn't started";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    if (!d.exists("backend")) {
        const char * msg = "missing target backend";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    if (!m_parent.backend_exists(d["backend"].as<string>())) {
        const char * msg = "unknown backend";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    PlatformDescription serialized;
    m_parent.serialize_backend_val(d, serialized);
    send_response<STA_OK, CMD_READ_BACKEND>(serialized);
}

void JsonConsoleClient::modify_generator(PlatformDescription &d)
{
    using namespace protocol;

    if (!d.exists("generator")) {
        const char * msg = "missing generator";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    const string name = d["generator"].as<string>();

    if (!m_parent.generator_exists(name)) {
        const char * msg = "unknown generator";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    SignalGenerator::Ptr gen = m_parent.get_generator(name);
    gen->reconfigure(d["params"]);

    send_response<STA_OK, CMD_MODIFY_GENERATOR>();
}

void JsonConsoleClient::modify_event(PlatformDescription &d)
{
    using namespace protocol;

    if (!d.exists("trigger")) {
        const char * msg = "missing trigger";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    const string name = d["trigger"].as<string>();

    if (!m_parent.event_exists(name)) {
        const char * msg = "unknown trigger";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    SignalEvent::Ptr gen = m_parent.get_event(name);
    gen->reconfigure(d["params"]);

    send_response<STA_OK, CMD_MODIFY_EVENT>();
}

void JsonConsoleClient::get_backend_status(PlatformDescription &d)
{
    using namespace protocol;

    if (!d.exists("backend")) {
        const char * msg = "missing target backend";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    const string name = d["backend"].as<string>();

    if (!m_parent.backend_exists(name)) {
        const char * msg = "unknown backend";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    const BackendInstance &b = m_parent.get_backend(name);
    send_response<STA_OK, CMD_GET_BACKEND_STATUS>(b.get_status(), b.get_failure_reason());
}

void JsonConsoleClient::get_generator_status(PlatformDescription &d)
{
    using namespace protocol;

    if (!d.exists("generator")) {
        const char * msg = "missing target generator";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    const string name = d["generator"].as<string>();

    if (!m_parent.generator_exists(name)) {
        const char * msg = "unknown generator";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    const SignalGenerator::Ptr &g = m_parent.get_generator(name);
    send_response<STA_OK, CMD_GET_GENERATOR_STATUS>(g->get_status(), g->get_failure_reason());
}

void JsonConsoleClient::get_event_status(PlatformDescription &d)
{
    using namespace protocol;

    if (!d.exists("trigger")) {
        const char * msg = "missing target trigger";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    const string name = d["trigger"].as<string>();

    if (!m_parent.event_exists(name)) {
        const char * msg = "unknown trigger";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    const SignalEvent::Ptr &e = m_parent.get_event(name);
    send_response<STA_OK, CMD_GET_EVENT_STATUS>(e->get_status(), e->get_failure_reason());
}

void JsonConsoleClient::delete_event(PlatformDescription &d)
{
    using namespace protocol;

    if (!d.exists("trigger")) {
        const char * msg = "missing target trigger";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    const string name = d["trigger"].as<string>();

    if (!m_parent.event_exists(name)) {
        const char * msg = "unknown trigger";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
        return;
    }

    m_parent.delete_event(name);
    send_response<STA_OK, CMD_DELETE_EVENT>();
}

void JsonConsoleClient::continue_elaboration()
{
    using namespace protocol;

    if (m_parent.get_simulation_status() == JsonConsolePlugin::BEFORE_ELABORATION) {
        m_parent.continue_elaboration();
        send_response<STA_OK, CMD_CONTINUE_ELABORATION>();
    } else {
        const char * msg = "elaboration already done";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
    }
}

void JsonConsoleClient::start_simulation()
{
    using namespace protocol;

    if (m_parent.get_simulation_status() == JsonConsolePlugin::BEFORE_SIMULATION) {
        m_parent.start_simulation();
        send_response<STA_OK, CMD_START_SIMULATION>();
    } else {
        const char * msg = "elaboration not done or simulation already started";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
    }
}

void JsonConsoleClient::resume_simulation()
{
    using namespace protocol;

    if (m_parent.get_simulation_status() == JsonConsolePlugin::SIMULATION_PAUSED) {
        m_parent.resume_simulation();
        send_response<STA_OK, CMD_RESUME_SIMULATION>();
    } else {
        const char * msg = "simulation is not in paused state";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
    }
}

void JsonConsoleClient::pause_simulation()
{
    using namespace protocol;

    if (m_parent.get_simulation_status() == JsonConsolePlugin::SIMULATION_RUNNING) {
        m_parent.pause_simulation(shared_from_this());
        send_response<STA_OK, CMD_PAUSE_SIMULATION>();
    } else {
        const char * msg = "simulation is not running";
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(msg);
    }
}

void JsonConsoleClient::handle_cmd(PlatformDescription &d)
{
    using namespace protocol;

    Command cmd = parse_command(d);

    try {
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
            continue_elaboration();
            break;
        case CMD_START_SIMULATION:
            start_simulation();
            break;
        case CMD_RESUME_SIMULATION:
            resume_simulation();
            break;
        case CMD_PAUSE_SIMULATION:
            pause_simulation();
            break;
        case CMD_ADD_BACKEND:
            add_backend(d);
            break;
        case CMD_ADD_GENERATOR:
            add_generator(d);
            break;
        case CMD_ADD_EVENT:
            add_event(d);
            break;
        case CMD_MODIFY_GENERATOR:
            modify_generator(d);
            break;
        case CMD_MODIFY_EVENT:
            modify_event(d);
            break;
        case CMD_GET_BACKEND_STATUS:
            get_backend_status(d);
            break;
        case CMD_GET_GENERATOR_STATUS:
            get_generator_status(d);
            break;
        case CMD_GET_EVENT_STATUS:
            get_event_status(d);
            break;
        case CMD_DELETE_EVENT:
            delete_event(d);
            break;
        case CMD_READ_BACKEND:
            read_backend(d);
            break;
        default:
            assert(false);
        }
    } catch (RabbitsException &e) {
        string reason("internal error: ");
        reason += e.what();
        send_response<STA_FAILURE, CMD_FAILURE_REASON>(reason.c_str());
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

void JsonConsoleClient::signal_event(SignalEvent &ev)
{
    using namespace protocol;
    send_response<STA_EVENT, CMD_TRIGGER>(ev.get_name());
}

void JsonConsoleClient::pause_event()
{
    using namespace protocol;
    send_response<STA_EVENT, CMD_SIMULATION_PAUSED>();
}

Logger & JsonConsoleClient::get_logger(LogContext::value context) const
{
    return m_parent.get_logger(context);
}
