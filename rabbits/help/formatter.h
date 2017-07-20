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

#include <string>
#include <vector>

#include <boost/tokenizer.hpp>

#include <rabbits/module/parameters.h>
#include <rabbits/module/namespace.h>
#include <rabbits/module/module.h>
#include <rabbits/logger.h>


static inline std::string get_param_full_name(const ParameterBase &param)
{
    std::string ret = "-" + param.get_namespace().get_name();

    if (param.get_module()) {
        ret += "." + param.get_module()->get_name();
    }

    ret += "." + param.get_name();

    return ret;
}

static inline std::string strip_last_nl(const std::string &s)
{
    std::string ret = s;

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

    void reset_pos()
    {
        if (m_cur > m_start_at) {
            if (!m_is_tty) {
                /* We can't go backward */
                return;
            }

            m_logger << "\r";
            m_cur = 0;
        }

        spaces(m_start_at - m_cur);
        m_cur = m_start_at;
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

        if (m_tree_enabled) {
            m_tree_trigger = true;
        }
    }

    void print_raw(const std::string &s)
    {
        m_logger << s;
        m_cur += s.size();
    }

    void print(const std::string &s)
    {
        if (!s.size()) {
            return;
        }

        Logger &l = m_logger;

        boost::char_separator<char> line_sep("\n");
        boost::char_separator<char> word_sep(" \t");

        Tokenizer lines_toks(s, line_sep);

        std::vector<std::string> lines;
        std::copy(lines_toks.begin(), lines_toks.end(), std::back_inserter(lines));

        if ((!lines.size()) && s.size()) {
            /* s contains \n only */
            for (unsigned int i = 0; i < s.size(); i++) {
                wrap();
            }
            return;
        }

        int nl_count = lines.size();
        if (s[s.size()-1] != '\n') {
            nl_count--;
        }

        if (!m_is_tty) {
            if (!m_cur && m_tree_enabled) {
                print_tree();
            }

            l << s;

            if (nl_count) {
                m_cur = 0;
            }

            if (s.back() != '\n') {
                m_cur += lines.back().size();
            }

            return;
        }

        for (auto line : lines) {
            Tokenizer words(line, word_sep);
            bool first = true;

            if (!m_cur && m_tree_enabled) {
                print_tree();
            }

            for (auto word : words) {
                if (first && line[0] == ' ') {
                    l << " ";
                    m_cur++;
                }

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

    typedef Logger& (*LoggerFormatFn)(Logger &);
    typedef std::pair<LoggerFormatFn, bool> TreeLvlInfo;

    bool m_tree_enabled = false;
    bool m_tree_trigger = false;
    std::vector<TreeLvlInfo> m_tree_lvls;

    void print_tree()
    {
        int i = 0;

        if (!m_tree_trigger) {
            return;
        }

        m_tree_trigger = false;

        for (auto &lvl : m_tree_lvls) {
            if (lvl.first == nullptr) {
                print_raw("    ");
            } else {
                m_logger << lvl.first;
                if (i == int(m_tree_lvls.size() - 1)) {
                    if (lvl.second) {
                        print_raw("\\__ ");
                    } else {
                        print_raw("|-- ");
                    }
                } else {
                    if (!lvl.second) {
                        print_raw("|   ");
                    } else {
                        print_raw("    ");
                    }
                }
            }

            m_logger << format::reset;
            i++;
        }

    }

public:
    TextFormatter(Logger &l, LogLevel::value lvl) : m_logger(l), m_lvl(lvl)
    {
        m_is_tty = l.is_tty(m_lvl);
        m_logger.next_trace(m_lvl);
        set_start_col(0);
    }

    void set_max_len(int len)
    {
        m_forced_max_len = len;
    }

    void set_start_col(int col, bool next_line = false)
    {
        if (next_line) {
            m_next_start_at = col;
            return;
        }

        int rows, cols;

        if (m_is_tty && !m_forced_max_len) {
            m_logger.get_tty_attr(m_lvl, rows, cols);
            m_max_len = cols;
        } else if (!m_is_tty) {
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

    void enable_tree() { m_tree_enabled = true; m_tree_trigger = false; }
    void disable_tree() { m_tree_lvls.clear(); m_tree_enabled = false; }

    void tree_push(LoggerFormatFn color, bool last_child = false)
    {
        m_tree_lvls.push_back(TreeLvlInfo(color, last_child));
        m_tree_trigger = true;
    }

    void tree_set_last_child() { m_tree_lvls.back().second = true; }

    void tree_pop() { m_tree_lvls.pop_back(); m_tree_trigger = true; }

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
        if (m_tree_enabled) {
            print_tree();
        }

        (*pf)(m_logger);
        return *this;
    }

    Logger & get_logger() { return m_logger; }
};

static inline void print_value(TextFormatter &f,
                               const std::string &key, const std::string &val)
{
    f << format::cyan << key << ": ";
    f.inc_start_col(2, true);
    f << format::reset << strip_last_nl(val) << "\n";
    f.dec_start_col(2);
}
