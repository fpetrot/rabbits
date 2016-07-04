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

#include <rabbits/component/factory.h>

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
    return "-" + param.get_namespace() + "." + param.get_name();
}

class TextFormatter {
public:
    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;

private:
    Logger &m_logger;
    LogLevel::value m_lvl;

    bool m_is_tty;
    int m_start_at, m_max_len;
    int m_cur;

    static string tabs(int c)
    {
        string ret;

        while(c--) {
            ret += "\t";
        }

        return ret;
    }

public:
    TextFormatter(Logger &l, LogLevel::value lvl) : m_logger(l), m_lvl(lvl)
    {
        m_is_tty = l.is_tty(m_lvl);
    }

    void reset_pos()
    {
        m_logger << "\r" << tabs(m_start_at);
        m_cur = m_start_at * 8;
    }

    void set_start_col(int col)
    {
        if (!m_is_tty) {
            return;
        }

        int rows, cols;

        m_logger.get_tty_attr(m_lvl, rows, cols);

        m_start_at = col / 8;
        if (col % 8) {
            m_start_at++;
        }

        m_max_len = cols - m_start_at * 8;

        /* XXX m_max_len <= 0 */
        reset_pos();
    }

    void wrap() {
        m_logger << "\n";
        reset_pos();
    }

    void print(const string &s)
    {
        if (!m_is_tty) {
            m_logger << s;
            return;
        }

        Logger &l = m_logger;

        boost::char_separator<char> line_sep("\n");
        boost::char_separator<char> word_sep(" \t");

        Tokenizer lines(s, line_sep);

        for (auto line : lines) {
            Tokenizer words(line, word_sep);

            for (auto word : words) {
                if (word.size() > m_max_len) {
                    l << word << " ";
                    wrap(); /* XXX */
                    continue;
                }

                if (m_cur + word.size() >= m_max_len) {
                    wrap();
                }

                l << word << " ";
                m_cur += word.size();
            }

            //l << "\n";
            //reset_pos();
        }
    }

    Logger & get_logger() { return m_logger; }
};

class UsageEntry {
public:
    virtual int left_indent() const = 0;
    virtual int left_length() const = 0;
    virtual int right_length() const = 0;
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
    int right_length() const { return 0; }

    void print_left(TextFormatter &f)
    {
        f.wrap();
        f.print(m_name + ":");
    }

    void print_right(TextFormatter &f)
    {
    }
};

class UsageEntryAlias : public UsageEntry {
private:
    string m_name, m_right;
    const ParameterBase &m_param;
public:
    UsageEntryAlias(const string &name, const ParameterBase &param) 
        : m_name(name), m_param(param)
    {
        m_right = m_param.get_description() 
            + " (shortcut for " + get_param_full_name(param) + ")";
    }

    int left_indent() const { return 2; }
    int left_length() const { return m_name.size() + 1; }
    int right_length() const { return m_right.size(); }

    void print_left(TextFormatter &f)
    {
        f.print("-" + m_name);
    }

    void print_right(TextFormatter &f)
    {
        f.print(m_param.get_description());

        f.get_logger() << format::black_b;
        f.print("(shortcut for " + get_param_full_name(m_param) + ")");

        f.get_logger() << format::reset;
    }
};

class UsageEntryParam : public UsageEntry {
    const ParameterBase &m_param;
    string m_left, m_right;

public:
    UsageEntryParam(const ParameterBase &param) : m_param(param)
    {
        m_left = get_param_full_name(param);
        m_right = param.get_description() + " [" + param.to_str() + "]";
    }

    int left_indent() const { return 2; }
    int left_length() const { return m_left.size(); }
    int right_length() const { return m_right.size(); }

    void print_left(TextFormatter &f)
    {
        f.print(m_left);
    }

    void print_right(TextFormatter &f)
    {
        f.print(m_param.get_description());

        f.get_logger() << format::cyan;
        f.print("[" + m_param.to_str() + "]");

        f.get_logger() << format::reset;

    }
};

class UsageFormatter {
public:
    const string LEFT_PREFIX = "  ";

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

        l.next_trace(lvl);

        for (auto &u : m_entries) {
            f.set_start_col(0);

            for (int i = 0; i < u->left_indent(); i++) {
                l << " ";
            }

            u->print_left(f);

            f.set_start_col(m_max_left + 3);
            u->print_right(f);

            l << "\n";
        }
    }
};

static string operator*(const string &s, int i)
{
    string ret;

    while(i--) {
        ret += s;
    }

    return ret;
}

const string INDENT = "   ";
static int indent_step = 0;
static Logger & cout_indent(int istep = indent_step)
{
    LOG(APP, INF) << (INDENT*istep);
    return get_app_logger();
}

static void describe_comp_params(const ComponentParameters &p)
{
    ComponentParameters::const_iterator it;

    for (it = p.begin(); it != p.end(); it++) {
        cout_indent()
            << "- " << it->first
            << ": " << it->second->get_description() << "\n";
    }
}

static void describe_component(ComponentFactory &c)
{
    const ComponentParameters & p = c.get_params();

    cout_indent() << "type: " << c.type() << "\n";
    cout_indent() << "description: " << c.description() << "\n";

    if (p.empty()) {
        return;
    }

    cout_indent() << "parameters:\n";
    indent_step++;
    describe_comp_params(p);
    indent_step--;
}

void enum_components()
{
    ComponentManager &cm = ComponentManager::get();
    ComponentManager::iterator it;

    cout_indent() << "\nAvailable components:\n\n";

    for (it = cm.begin(); it != cm.end(); it++) {
        cout_indent() << "* " << it->first << "\n";
        indent_step++;
        describe_component(*(it->second));
        indent_step--;
        cout_indent() << "\n";
    }
}

static void print_option(const string &opt, const string& help)
{
    cout_indent(1) << "-" << opt << "\r\t\t\t\t\t" << help << "\n";
}

static void print_option_with_val(const string &opt, const string& help, const string &value)
{
    cout_indent(1) << "-" << opt << "\r\t\t\t\t\t" << help << " [" << value << "]\n";
}

static void print_option_with_alias(const string &opt, const string &help, const string &alias)
{
    cout_indent(1) << "-" << opt << "\r\t\t\t\t\t" << help << " (shortcut for -" << alias << ")\n";
}


static void print_param(const ParameterBase &param)
{
    const string n = param.get_namespace() + "." + param.get_name();
    print_option_with_val(n, param.get_description(), param.to_str());
}


static void print_comp_usage(const string &comp, const ComponentParameters &p)
{
    ComponentParameters::const_iterator it;

    for (it = p.begin(); it != p.end(); it++) {
        print_option_with_val("components." + comp + "." + it->first,
                              it->second->get_description(), it->second->to_str());
    }
}

static void add_aliases(ConfigManager &conf, UsageFormatter &usage)
{
    const ConfigManager::ParamAliases &aliases = conf.get_param_aliases();
    usage.add_section("Shortcuts");

    for (const auto alias : aliases) {
        usage.add_alias(alias.first, *alias.second);
    }
}

static void add_global_parameters(ConfigManager &conf, UsageFormatter &usage)
{
    const ComponentParameters & globals = conf.get_global_params();
    usage.add_section("Rabbits global parameters");

    for (const auto param : globals) {
        usage.add_param(*param.second);
    }
}

static void add_platform_parameters(PlatformBuilder &p, UsageFormatter &usage)
{
    if (p.is_empty()) {
        return;
    }

    usage.add_section("Platform parameters");
    for (auto it = p.comp_begin(); it != p.comp_end(); it++) {
        for (auto param : it->second->get_params()) {
            usage.add_param(*param.second);
        }
    }
}

void print_usage(const char* arg0, ConfigManager &conf, PlatformBuilder &p)
{
    UsageFormatter usage;

    bool banner_status = get_app_logger().enable_banner(false);

    LOG(APP, INF) << "Usage: " << arg0 << " [...]\n";

    add_aliases(conf, usage);
    add_global_parameters(conf, usage);
    add_platform_parameters(p, usage);

    usage.dump(LogLevel::INFO);

    get_app_logger().enable_banner(banner_status);
}

void print_version(ostream &s)
{
    s << RABBITS_APP_NAME
        << " version " << RABBITS_VERSION
        << " api version " << RABBITS_API_VERSION << "\n";
}
