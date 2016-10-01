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
#include <cmath>
#include <sstream>

#include <systemc>

#include "rabbits/test/comparator.h"
#include "rabbits/component/component.h"
#include "rabbits/rabbits_exception.h"
#include "rabbits/logger.h"
#include "rabbits/config/has_config.h"

class TestBase;

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

    virtual TestBase * create(ConfigManager &) const = 0;
};

class TestBase {
private:
    std::string m_name;
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

    void set_test_failed() { m_test_result = false; }

    void failure(const std::string &what)
    {
        throw TestFailureException(m_name, what, get_current_file(), get_current_line());
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
    virtual bool more_test_check() const { return true; }

public:
    TestBase(const std::string &n, ConfigManager &c)
        : m_name(n), m_config(c)
    {}

    virtual ~TestBase() {}

    bool tests_passed() const { return m_test_result && more_test_check(); }
    ConfigManager & get_config() const { return m_config; }

    virtual void run() = 0;

    const std::string & get_name() const { return m_name; }
};

class Test : public TestBase {
public:
    Test(const std::string &n, ConfigManager &c)
        : TestBase(n, c)
    {}

    void run()
    {
        unit();
    }
};


class TestBench : public TestBase, public sc_core::sc_module {
public:
    enum eTestMode {
        REACH_THE_END,
        MAY_NOT_REACH_THE_END,
        DOES_NOT_REACH_THE_END
    };

protected:

    eTestMode m_test_mode = REACH_THE_END;
    bool m_end_reached = false;

    bool more_test_check() const {
         if (m_test_mode == REACH_THE_END && !m_end_reached) {
             LOG(APP, ERR) << "The test has not reached the end\n";
             return false;
         }

         if (m_test_mode == DOES_NOT_REACH_THE_END && m_end_reached) {
             LOG(APP, ERR) << "The test has reached the end\n";
             return false;
         }

         return true;
    }

    void set_test_mode(eTestMode m) { m_test_mode = m; }

    void unit_wrapper()
    {
        try {
            unit();
        } catch (TestFailureException e) {
            set_test_failed();
            LOG(APP, ERR) << e.what() << "\n";
        }

        m_end_reached = true;
    }

public:
    SC_HAS_PROCESS(TestBench);
    TestBench(sc_core::sc_module_name n, ConfigManager &c)
        : TestBase(std::string(n), c)
        , sc_core::sc_module(n)
    {
        SC_THREAD(unit_wrapper);
    }

    void run()
    {
        sc_core::sc_start();
    }
};

#ifndef RABBITS_TEST_MOD
# define RABBITS_TEST_MOD
# define RABBITS_TEST_MOD_empty
#endif

#define RABBITS_TEST___factory_name(_mod, _name) Test_ ## _mod ## _ ## _name ## _Factory
#define RABBITS_TEST__factory_name(_mod, _name) RABBITS_TEST___factory_name(_mod, _name)
#define RABBITS_TEST_factory_name(_name) RABBITS_TEST__factory_name(RABBITS_TEST_MOD, _name)

#define RABBITS_TEST___name(_mod, _name) Test_ ## _mod ## _ ## _name
#define RABBITS_TEST__name(_mod, _name) RABBITS_TEST___name(_mod, _name)
#define RABBITS_TEST_name(_name) RABBITS_TEST__name(RABBITS_TEST_MOD, _name)

#define RABBITS_TEST___inst(_mod, _name) _mod ## _ ## _name ## _inst
#define RABBITS_TEST__inst(_mod, _name) RABBITS_TEST___inst(_mod, _name)
#define RABBITS_TEST_inst(_name) RABBITS_TEST__inst(RABBITS_TEST_MOD, _name)

#define RABBITS_TEST_strify(a) # a

#ifdef RABBITS_TEST_MOD_empty
# define RABBITS_TEST___str(mod, name) RABBITS_TEST_strify(name)
#else
# define RABBITS_TEST___str(mod, name) RABBITS_TEST_strify(mod) ":" RABBITS_TEST_strify(name)
#endif

#define RABBITS_TEST__str(mod, name) RABBITS_TEST___str(mod, name)
#define RABBITS_TEST_str(name) RABBITS_TEST__str(RABBITS_TEST_MOD, name)

#define RABBITS_GEN_TEST_FACTORY(_name)                                       \
    class RABBITS_TEST_factory_name(_name)                                    \
        : public TestFactory {                                                \
    public:                                                                   \
        RABBITS_TEST_factory_name(_name)(std::string n)                       \
            : TestFactory(n) {}                                               \
        virtual TestBase * create(ConfigManager &c) const {                   \
            return new RABBITS_TEST_name(_name) (RABBITS_TEST_str(_name), c); \
        }                                                                     \
    };                                                                        \
    static RABBITS_TEST_factory_name(_name)                                   \
    RABBITS_TEST_inst(_name)(RABBITS_TEST_str(_name));                        \


#define RABBITS_UNIT_TEST(_name)                                  \
    class RABBITS_TEST_name(_name) : public Test {                \
    public:                                                       \
        RABBITS_TEST_name(_name)(std::string n, ConfigManager &c) \
            : Test(n, c) {}                                       \
        virtual void unit();                                      \
    };                                                            \
    RABBITS_GEN_TEST_FACTORY(_name)                               \
    void RABBITS_TEST_name(_name)::unit()


#define RABBITS_UNIT_TESTBENCH(_name, _base)                 \
    class RABBITS_TEST_name(_name) : public _base {          \
    public:                                                  \
        RABBITS_TEST_name(_name)(sc_core::sc_module_name n,  \
                                 ConfigManager &c)           \
            : _base(n, c) {}                                 \
        virtual void unit();                                 \
    };                                                       \
    RABBITS_GEN_TEST_FACTORY(_name)                          \
    void RABBITS_TEST_name(_name)::unit()

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

#define RABBITS_TEST_ASSERT_TIME_DELTA_ERR(expected, error)                             \
    do {                                                                                \
        sc_core::sc_time expected_delta(expected);                                      \
        sc_core::sc_time effective_delta = sc_core::sc_time_stamp() - m_last_timestamp; \
        double diff = abs(expected_delta.to_double() - effective_delta.to_double());    \
        RABBITS_TEST_ASSERT_LT(diff, error.to_double());                                \
        m_last_timestamp = sc_core::sc_time_stamp();                                    \
    } while(0)

#define RABBITS_TEST_END() \
    sc_core::sc_stop()

#define RABBITS_GET_TEST_DIR() \
    get_test_dir(__FILE__)

#endif
