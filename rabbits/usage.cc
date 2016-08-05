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

#include <rabbits/logger.h>
#include <rabbits/simu.h>

#include <rabbits/platform/description.h>
#include <rabbits/platform/builder.h>
#include <rabbits/platform/parser.h>

#include <rabbits/component/factory.h>
#include <rabbits/plugin/plugin.h>

#include <rabbits/ui/ui.h>

#include <iostream>
#include <set>
#include <list>
#include <memory>

#include <boost/tokenizer.hpp>

#include "usage.h"

using std::string;
using std::vector;
using std::set;
using std::list;
using std::ostream;
using std::unique_ptr;

static inline string get_param_full_name(const ParameterBase &param)
{
    string ret = "-" + param.get_namespace().get_name();

    if (param.get_module()) {
        ret += "." + param.get_module()->get_name();
    }

    ret += "." + param.get_name();

    return ret;
}

static inline string strip_last_nl(const string &s)
{
    string ret = s;

    if (ret[ret.size()-1] == '\n') {
        ret.resize(ret.size()-1);
    }

    return ret;
}

class TextFormatter {
public:
    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;

private:
    Logger &m_logger;
    LogLevel::value m_lvl;

    bool m_is_tty;
    int m_start_at, m_max_len;
    int m_next_start_at = -1;
    int m_forced_max_len = 0;
    int m_cur = 0;

    void spaces(int c)
    {
        while(c--) {
            m_logger << " ";
        }
    }


public:
    TextFormatter(Logger &l, LogLevel::value lvl) : m_logger(l), m_lvl(lvl)
    {
        m_is_tty = l.is_tty(m_lvl);
        m_logger.next_trace(m_lvl);
        set_start_col(0);
    }

    void reset_pos()
    {
        if (m_cur > m_start_at) {
            m_logger << "\r";
            m_cur = 0;
        }

        spaces(m_start_at - m_cur);
        m_cur = m_start_at;
    }

    void set_max_len(int len)
    {
        m_forced_max_len = len;
    }

    void set_start_col(int col, bool next_line = false)
    {
        if (!m_is_tty) {
            return;
        }

        if (next_line) {
            m_next_start_at = col;
            return;
        }

        int rows, cols;

        if (!m_forced_max_len) {
            m_logger.get_tty_attr(m_lvl, rows, cols);
            m_max_len = cols;
        } else {
            m_max_len = m_forced_max_len;
        }

        m_start_at = col;

        /* XXX m_max_len <= 0 */
        reset_pos();
    }

    void inc_start_col(int inc, bool next_line = false)
    {
        set_start_col(m_start_at + inc, next_line);
    }

    void dec_start_col(int dec, bool next_line = false)
    {
        set_start_col(m_start_at - dec, next_line);
    }

    void wrap() {
        m_logger << "\n";
        m_cur = 0;

        if (m_next_start_at >= 0) {
            set_start_col(m_next_start_at, false);
            m_next_start_at = -1;
        } else if (m_is_tty) {
            reset_pos();
        }
    }

    void print(const string &s)
    {
        if (!m_is_tty) {
            m_logger << s << " ";
            return;
        }

        Logger &l = m_logger;

        boost::char_separator<char> line_sep("\n");
        boost::char_separator<char> word_sep(" \t");

        Tokenizer lines_toks(s, line_sep);

        std::vector<string> lines;
        std::copy(lines_toks.begin(), lines_toks.end(), std::back_inserter(lines));

        if ((!lines.size()) && s.size()) {
            /* \n only */
            for (unsigned int i = 0; i < s.size(); i++) {
                wrap();
            }
            return;
        }

        int nl_count = lines.size();
        if (s[s.size()-1] != '\n') {
            nl_count--;
        }

        for (auto line : lines) {
            Tokenizer words(line, word_sep);
            bool first = true;

            for (auto word : words) {
                if (int(word.size()) > m_max_len) {
                    l << word;
                    wrap();
                    continue;
                }

                if (m_cur + int(word.size()) >= m_max_len) {
                    wrap();
                    first = true;
                }

                if (!first) {
                    l << " ";
                    m_cur++;
                }

                l << word;
                m_cur += word.size();

                first = false;
            }

            if (line[line.size()-1] == ' ') {
                l << " ";
                m_cur++;
            }

            if (nl_count--) {
                wrap();
            }
        }
    }

    template <class T>
    TextFormatter & operator << (const T& t)
    {
        std::stringstream ss;

        ss << t;
        print(ss.str());

        return *this;
    }

    TextFormatter & operator << (Logger & (*pf)(Logger &))
    {
        (*pf)(m_logger);
        return *this;
    }

    Logger & get_logger() { return m_logger; }
};

class UsageEntry {
public:
    virtual int left_indent() const = 0;
    virtual int left_length() const = 0;
    virtual void print_left(TextFormatter &f) = 0;
    virtual void print_right(TextFormatter &f) = 0;
};

class UsageEntrySection : public UsageEntry {
private:
    string m_name;

public:
    UsageEntrySection(const string & name) : m_name(name) {}
    int left_indent() const { return 0; }
    int left_length() const { return m_name.size() + 1; }

    void print_left(TextFormatter &f)
    {
        f.wrap();
        f << format::white_b << m_name << ":" << format::reset;
    }

    void print_right(TextFormatter &f) {}
};

class UsageEntryAlias : public UsageEntry {
private:
    string m_name;
    const ParameterBase &m_param;
public:
    UsageEntryAlias(const string &name, const ParameterBase &param)
        : m_name(name), m_param(param)
    {}

    int left_indent() const { return 2; }
    int left_length() const
    {
        int ret = m_name.size() + 1;

        if (!m_param.is_convertible_to<bool>()) {
            ret += 3 + std::strlen(m_param.get_typeid());
        }

        return ret;
    }

    void print_left(TextFormatter &f)
    {
        f << "-"
          << (m_param.is_advanced() ? format::yellow : format::white)
          << m_name;

        if (!m_param.is_convertible_to<bool>()) {
            f << " " << format::cyan << "<" << m_param.get_typeid() << ">";
        }

        f << format::reset;
    }

    void print_right(TextFormatter &f)
    {
        f << m_param.get_description() << " "
          << format::black_b << "(shortcut for " << get_param_full_name(m_param) << ")"
          << format::reset;
    }
};

class UsageEntryParam : public UsageEntry {
    const ParameterBase &m_param;
    string m_left;

public:
    UsageEntryParam(const ParameterBase &param) : m_param(param)
    {
        m_left = get_param_full_name(param) + " <>" + m_param.get_typeid();
    }

    int left_indent() const { return 2; }
    int left_length() const { return m_left.size(); }

    void print_left(TextFormatter &f)
    {
        f << "-" << m_param.get_namespace().get_name();

        if (m_param.get_module()) {
            f << "." << m_param.get_module()->get_name();
        }

        f << "." << (m_param.is_advanced() ? format::yellow : format::green) << m_param.get_name() << " "
          << format::cyan << "<" << m_param.get_typeid() << ">"
          << format::reset;
    }

    void print_right(TextFormatter &f)
    {
        string description;

        description = strip_last_nl(m_param.get_description());

        f << description << " "
          << format::cyan << "[" << m_param.to_str() << "]"
          << format::reset;

    }
};

class UsageFormatter {
private:
    list< unique_ptr<UsageEntry> > m_entries;
    int m_max_left = 0;

    void add_usage(UsageEntry *u)
    {
        m_entries.push_back(unique_ptr<UsageEntry>(u));
        m_max_left = std::max(m_max_left, u->left_length());
    }

public:
    void add_section(const string &name) { add_usage(new UsageEntrySection(name)); }
    void add_param(const ParameterBase &param) { add_usage(new UsageEntryParam(param)); }
    void add_alias(const string &name, const ParameterBase &param) { add_usage(new UsageEntryAlias(name, param)); }

    void dump(LogLevel::value lvl) {
        Logger &l = get_app_logger();
        TextFormatter f(l, lvl);

        for (auto &u : m_entries) {
            f.set_start_col(u->left_indent());
            u->print_left(f);

            f.set_start_col(m_max_left + 5);
            u->print_right(f);

            l << "\n";
        }
    }
};

static void print_tree(std::vector<bool> lvls, TextFormatter &f, bool separator)
{
    f.set_start_col(1);
    int i = 0;

    for (bool lvl : lvls) {

        switch (i) {
        case 0:
            f << format::red;
            break;
        case 1:
            f << format::cyan;
            break;
        default:
            f << format::white;
        }

        if (i == int(lvls.size() - 1) && !separator) {
            if (lvl) {
                f << "\\__ ";
            } else {
                f << "|-- ";
            }
        } else {
            if (!lvl) {
                f << "|";
            }
        }

        f << format::reset;
        i++;
        f.inc_start_col(4);
    }
}


static void _dump_systemc_hierarchy(const sc_core::sc_object &top_level, TextFormatter &f, vector<bool> &lvls)
{
    const vector<sc_core::sc_object*> & children = top_level.get_child_objects();

    f << (lvls.size() ? ((lvls.size() > 1) ? format::white : format::cyan) : format::red) << top_level.basename()
      << format::reset << ":"
      << format::black << top_level.kind()
      << format::reset << "\n";

    for (auto &c : children) {
        const bool last = (&c == &(children[children.size()-1]));

        lvls.push_back(last);

        print_tree(lvls, f, false);

        _dump_systemc_hierarchy(*c, f, lvls);

        lvls.pop_back();

        if (last) {
            print_tree(lvls, f, true);
            f << "\n";
        }
    }
}

void dump_systemc_hierarchy(PlatformBuilder &p, LogLevel::value lvl)
{
    Logger &l = get_app_logger();
    vector<bool> lvls;

    bool banner_status = l.enable_banner(false);
    l << format::white_b << "SystemC hierarchy:\n\n" << format::reset;

    TextFormatter f(l, lvl);

    _dump_systemc_hierarchy(p, f, lvls);

    l << "\n";
    l.enable_banner(banner_status);
}


static void describe_comp_params(const Parameters &p, TextFormatter &f)
{
    Parameters::const_iterator it;

    for (it = p.begin(); it != p.end(); it++) {
        f.set_start_col(4);
        f << format::green << it->first << format::reset;

        f.set_start_col(5 + it->first.size());
        f << strip_last_nl(it->second->get_description()) << "\n";
    }
}

static inline void print_value(TextFormatter &f,
                               const string &key, const string &val)
{
    f << format::cyan << key << ": ";
    f.inc_start_col(2, true);
    f << format::reset << strip_last_nl(val) << "\n";
    f.dec_start_col(2);
}

static void describe_module(ModuleFactoryBase &c, TextFormatter &f)
{
    const Parameters & p = c.get_params();
    ModuleFactoryBase::ExtraValues values;

    f.set_max_len(80);

    c.get_extra_values(values);
    for (auto &v : values) {
        f.set_start_col(2);
        print_value(f, v.first, v.second);
    }

    f.set_start_col(2);
    print_value(f, "description", c.get_description());

    if (p.empty()) {
        return;
    }

    f.set_start_col(2);
    f << format::cyan << "parameters:\n" << format::reset;
    describe_comp_params(p, f);
}

void enum_modules(ConfigManager &config, const Namespace &ns, LogLevel::value lvl)
{
    ModuleManagerBase &man = config.get_manager_by_namespace(ns);

    Logger &l = get_app_logger();

    bool banner_status = l.enable_banner(false);

    TextFormatter f(l, lvl);

    f << format::white_b << "Available " << ns.get_name() << ":\n\n"
      << format::reset;

    for (auto &m : man) {
        f.set_start_col(0);
        f << format::cyan_b << "* "
          << format::white_b << m.first
          << format::reset << "\n";

        describe_module(*(m.second), f);

        f << "\n";
    }

    l << "\n";
    l.enable_banner(banner_status);
}

static void enum_platform(PlatformParser &parser, TextFormatter &f)
{
    f.set_start_col(2);
    print_value(f, "description", parser.get_root().get_description());

    if (parser.get_root().has_parent()) {
        print_value(f, "inherits from", parser.get_root().get_parent_name());
    }

    if (parser.get_root().is_generic()) {
        f << format::green << "This is a generic platform\n";
    }


    f << "\n";
}

void enum_platforms(ConfigManager &config, LogLevel::value lvl)
{
    Logger &l = get_app_logger();

    bool banner_status = l.enable_banner(false);

    TextFormatter f(l, lvl);

    f << format::white_b << "Available platforms:\n\n"
      << format::reset;

    for (auto p : config.get_platforms()) {
        f.set_start_col(0);
        f << format::cyan_b << "* "
          << format::white_b << p.first
          << format::reset << "\n";

        try {
            PlatformParser parser(p.first, p.second, config);
            enum_platform(parser, f);
        } catch (PlatformParseException e) {
            f.set_start_col(2);
            f << format::red_b << "Parsing error: " 
              << format::red << e.what() << "\n";
        }
    }

    l << "\n";
    l.enable_banner(banner_status);
}

static void add_aliases(ConfigManager &conf, UsageFormatter &usage, bool advanced)
{
    const ConfigManager::ParamAliases &aliases = conf.get_param_aliases();
    usage.add_section("Shortcuts");

    for (const auto alias : aliases) {
        if (advanced || !alias.second->is_advanced()) {
            usage.add_alias(alias.first, *alias.second);
        }
    }
}

static void add_parameters(const Parameters &params, UsageFormatter &usage,
                           bool advanced)
{
    for (const auto & param : params) {
        if (advanced || !param.second->is_advanced()) {
            usage.add_param(*param.second);
        }
    }
}

static void add_global_parameters(ConfigManager &conf, UsageFormatter &usage,
                                  bool advanced)
{
    const Parameters & globals = conf.get_global_params();
    usage.add_section("Rabbits global parameters");

    add_parameters(globals, usage, advanced);
}

static void add_platform_parameters(PlatformBuilder &p, UsageFormatter &usage,
                                    bool advanced)
{
    if (p.is_empty()) {
        return;
    }

    usage.add_section("Platform parameters");
    for (auto & comp : p.get_components()) {
        add_parameters(comp.second->get_params(), usage, advanced);
    }

    for (auto & plug : p.get_plugins()) {
        add_parameters(plug.second->get_params(), usage, advanced);
    }

    for (auto & be : p.get_backends()) {
        add_parameters(be.second->get_params(), usage, advanced);
    }

}

void print_usage(const char* arg0, ConfigManager &conf, PlatformBuilder &p)
{
    UsageFormatter usage;

    bool banner_status = get_app_logger().enable_banner(false);
    bool advanced = conf.get_global_params()["show-advanced-params"].as<bool>();

    LOG(APP, INF) << format::white_b << "Usage: " << arg0 << " [...]\n\n";

    if (advanced) {
        LOG(APP, INF) << format::yellow << "Displaying advanced parameters\n" << format::reset;
    }

    add_aliases(conf, usage, advanced);
    add_global_parameters(conf, usage, advanced);
    add_platform_parameters(p, usage, advanced);

    usage.dump(LogLevel::INFO);

    get_app_logger().enable_banner(banner_status);
}

void print_version(LogLevel::value lvl)
{
    Logger &l = get_app_logger();
    bool banner_status = l.enable_banner(false);

    l.next_trace(lvl);

    l << RABBITS_APP_NAME
        << " version " << RABBITS_VERSION
        << " api version " << RABBITS_API_VERSION << "\n";

    l.enable_banner(banner_status);
}
