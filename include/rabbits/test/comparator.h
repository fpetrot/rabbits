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

#ifndef _RABBITS_TEST_COMPARATOR_H
#define _RABBITS_TEST_COMPARATOR_H

#include <string>

namespace test {

enum comp_operator {
    eq, ne, lt, le, gt, ge
};

template <comp_operator T> struct comparator;

template <>
struct comparator<eq> {
    template <typename T1, typename T2>
    static bool compare(T1 a, T2 b) {
        return a == b;
    };

    static const std::string op_str() { return "=="; }
    static const std::string not_op_str() { return "!="; }
};

template <>
struct comparator<ne> {
    template <typename T1, typename T2>
    static bool compare(T1 a, T2 b) {
        return a != b;
    };

    static const std::string op_str() { return "!="; }
    static const std::string not_op_str() { return "=="; }
};

template <>
struct comparator<lt> {
    template <typename T1, typename T2>
    static bool compare(T1 a, T2 b) {
        return a < b;
    };

    static const std::string op_str() { return "<"; }
    static const std::string not_op_str() { return ">="; }
};

template <>
struct comparator<le> {
    template <typename T1, typename T2>
    static bool compare(T1 a, T2 b) {
        return a <= b;
    };

    static const std::string op_str() { return "<="; }
    static const std::string not_op_str() { return ">"; }
};

template <>
struct comparator<gt> {
    template <typename T1, typename T2>
    static bool compare(T1 a, T2 b) {
        return a < b;
    };

    static const std::string op_str() { return ">"; }
    static const std::string not_op_str() { return "<="; }
};

template <>
struct comparator<ge> {
    template <typename T1, typename T2>
    static bool compare(T1 a, T2 b) {
        return a <= b;
    };

    static const std::string op_str() { return ">="; }
    static const std::string not_op_str() { return "<"; }
};

};
#endif
