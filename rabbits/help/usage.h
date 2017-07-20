/*
 *  This file is part of Rabbits
 *  Copyright (C) 2017 Luc MICHEL - Antfield SAS
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

#include <list>
#include <memory>

#include "formatter.h"

class UsageEntry {
public:
    virtual int left_indent() const = 0;
    virtual int left_length() const = 0;
    virtual void print_left(TextFormatter &f) = 0;
    virtual void print_right(TextFormatter &f) = 0;
};

class UsageEntrySection : public UsageEntry {
private:
    std::string m_name;

public:
    UsageEntrySection(const std::string & name) : m_name(name) {}
    int left_indent() const { return 0; }
    int left_length() const { return m_name.size() + 1; }

    void print_left(TextFormatter &f)
    {
        f << "\n" << format::white_b << m_name << ":" << format::reset;
    }

    void print_right(TextFormatter &f) {}
};

class UsageEntryAlias : public UsageEntry {
private:
    std::string m_name;
    const ParameterBase &m_param;
public:
    UsageEntryAlias(const std::string &name, const ParameterBase &param)
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
    std::string m_left;

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
        std::string description;

        description = strip_last_nl(m_param.get_description());

        f << description << " "
          << format::cyan << "[" << m_param.to_str() << "]"
          << format::reset;

    }
};

class UsageFormatter {
private:
    std::list< std::unique_ptr<UsageEntry> > m_entries;
    int m_max_left = 0;

    void add_usage(UsageEntry *u)
    {
        m_entries.push_back(std::unique_ptr<UsageEntry>(u));
        m_max_left = std::max(m_max_left, u->left_length());
    }

public:
    void add_section(const std::string &name) { add_usage(new UsageEntrySection(name)); }
    void add_param(const ParameterBase &param) { add_usage(new UsageEntryParam(param)); }
    void add_alias(const std::string &name, const ParameterBase &param) { add_usage(new UsageEntryAlias(name, param)); }

    void dump(LogLevel::value lvl) {
        Logger &l = get_app_logger();
        TextFormatter f(l, lvl);

        for (auto &u : m_entries) {
            f.set_start_col(u->left_indent());
            u->print_left(f);

            f.set_start_col(m_max_left + 5);
            u->print_right(f);

            f << "\n";
        }

        l << "\n";
    }
};
