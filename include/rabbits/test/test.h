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

#ifndef _RABBITS_TEST_TEST_H
#define _RABBITS_TEST_TEST_H

#include <set>
#include <cstring>
#include <sstream>

#include <systemc>

#include "rabbits/test/comparator.h"
#include "rabbits/component/component.h"
#include "rabbits/rabbits_exception.h"
#include "rabbits/logger.h"
#include "rabbits/config/has_config.h"

class Test;

class TestFailureException : public RabbitsException {
protected:
    std::string build_what(const std::string &testname,
                           const std::string &what,
                           const std::string &filename,
                           int line) {
        std::stringstream ss;

        ss << testname << ": " << what << " at " << filename << ":" << line;
        return ss.str();
    }

public:
    TestFailureException(const std::string &what) : RabbitsException(what) {}
    TestFailureException(const std::string &testname,
                         const std::string &what,
                         const std::string &filename,
                         int line)
        : RabbitsException(build_what(testname, what, filename, line)) {}

    virtual ~TestFailureException() throw() {}
};

class TestFactory {
public:
    typedef std::set<TestFactory*>::const_iterator const_iterator;

private:
    static std::set<TestFactory*> *m_insts;

    static void register_test(TestFactory* t) {
        if (m_insts == NULL) {
            m_insts = new std::set<TestFactory*>;
        }

        m_insts->insert(t);
    }

    static void unregister_test(TestFactory* t) {
        if (m_insts == NULL) {
            m_insts = new std::set<TestFactory*>;
        }

        m_insts->erase(t);
    }

protected:
    std::string m_name;

public:
    static const_iterator begin() {
        if (m_insts == NULL) {
            m_insts = new std::set<TestFactory*>;
        }
        return m_insts->begin();
    }

    static const_iterator end() {
        if (m_insts == NULL) {
            m_insts = new std::set<TestFactory*>;
        }
        return m_insts->end();
    }

    TestFactory(const std::string & test_name)
        : m_name(test_name)
    {
        register_test(this);
    }

    ~TestFactory() {
        unregister_test(this);
    }

    const std::string & get_name() { return m_name; }

    virtual Test * create(ConfigManager &) const = 0;
};

class Test : public sc_core::sc_module {
private:
    bool m_test_result = true;
    std::string m_current_filename = "??";
    int m_current_line = -1;
    ConfigManager & m_config;

protected:
    sc_core::sc_time m_last_timestamp = sc_core::SC_ZERO_TIME;

    void set_current_file(const std::string &fn) { m_current_filename = fn; }
    void set_current_line(int line) { m_current_line = line; }
    const std::string & get_current_file() { return m_current_filename; }
    int get_current_line() { return m_current_line; }
    std::string get_test_dir(const std::string &fn) const;

    void failure(const std::string &what)
    {
        throw TestFailureException(name(), what, get_current_file(), get_current_line());
    }

    ComponentBase * create_component_by_implem(const std::string name, const std::string yml_params);

    void test_assert(bool assertion, const std::string &lit_assert)
    {
        if (!assertion) {
            failure("`" + lit_assert + "' failed");
        }
    }

    template <test::comp_operator OP, typename T1, typename T2>
    void test_compare(T1 a, T2 b,
                      const std::string &lit_a, const std::string &lit_b)
    {
        if (!(test::comparator<OP>::compare(a, b))) {
            std::stringstream ss;

            ss << lit_a << " " << test::comparator<OP>::not_op_str() << " " << lit_b
                << " (" << lit_a << "=" << a
                << ", " << lit_b << "=" << b << ")";

            failure(ss.str());
        }
    }

    virtual void unit() = 0;

    void unit_wrapper()
    {
        try {
            unit();
        } catch (TestFailureException e) {
            m_test_result = false;
            LOG(APP, ERR) << e.what_without_bt() << "\n";
        }
    }

public:
    SC_HAS_PROCESS(Test);
    Test(sc_core::sc_module_name n, ConfigManager &c)
        : sc_module(n)
        , m_config(c)
    {
        SC_THREAD(unit_wrapper);
    }

    bool tests_passed() const { return m_test_result; }
    ConfigManager & get_config() const { return m_config; }
};

#define RABBITS_UNIT_TEST(_name, _base)                             \
    class Test_ ## _name : public _base {                           \
    public:                                                         \
        SC_HAS_PROCESS(Test_ ## _name);                             \
        Test_ ## _name(sc_core::sc_module_name n, ConfigManager &c) \
            : _base(n, c) {}                                        \
        virtual void unit();                                        \
    };                                                              \
    class Test_ ## _name ## _Factory : public TestFactory {         \
    public:                                                         \
        Test_ ## _name ## _Factory(std::string n)                   \
            : TestFactory(n) {}                                     \
        virtual Test * create(ConfigManager &c) const {             \
            return new Test_ ## _name (# _name, c);                 \
        }                                                           \
    };                                                              \
    static Test_ ## _name ## _Factory _name ## _inst(# _name);      \
    void Test_ ## _name::unit()

#define RABBITS_TEST_ASSERT(assertion)      \
    do {                                    \
        set_current_file(__FILE__);         \
        set_current_line(__LINE__);         \
        test_assert(assertion, #assertion); \
    } while(0)

#define RABBITS_TEST_COMPARE(op, a, b)        \
    do {                                      \
        set_current_file(__FILE__);           \
        set_current_line(__LINE__);           \
        test_compare<test::op>(a, b, #a, #b); \
    } while(0)

#define RABBITS_TEST_ASSERT_EQ(a, b) \
    RABBITS_TEST_COMPARE(eq, a, b)
#define RABBITS_TEST_ASSERT_NE(a, b) \
    RABBITS_TEST_COMPARE(ne, a, b)
#define RABBITS_TEST_ASSERT_LT(a, b) \
    RABBITS_TEST_COMPARE(lt, a, b)
#define RABBITS_TEST_ASSERT_LE(a, b) \
    RABBITS_TEST_COMPARE(le, a, b)
#define RABBITS_TEST_ASSERT_GT(a, b) \
    RABBITS_TEST_COMPARE(gt, a, b)
#define RABBITS_TEST_ASSERT_GE(a, b) \
    RABBITS_TEST_COMPARE(ge, a, b)

#define RABBITS_TEST_ASSERT_TIME(expected)                        \
    do {                                                          \
        sc_core::sc_time expected_time(expected);                 \
        sc_core::sc_time current_time = sc_core::sc_time_stamp(); \
        RABBITS_TEST_ASSERT_EQ((expected_time, current_time)      \
    } while(0)

#define RABBITS_TEST_ASSERT_TIME_DELTA(expected)                                        \
    do {                                                                                \
        sc_core::sc_time expected_delta(expected);                                      \
        sc_core::sc_time effective_delta = sc_core::sc_time_stamp() - m_last_timestamp; \
        RABBITS_TEST_ASSERT_EQ(expected_delta, effective_delta);                        \
        m_last_timestamp = sc_core::sc_time_stamp();                                    \
    } while(0)


#define RABBITS_TEST_END() \
    sc_core::sc_stop()

#define RABBITS_GET_TEST_DIR() \
    get_test_dir(__FILE__)

#endif
