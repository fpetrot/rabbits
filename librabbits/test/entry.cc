#include "rabbits/test/test.h"
#include "rabbits/logger.h"
#include "rabbits/component/manager.h"
#include "rabbits/component/factory.h"
#include "rabbits/platform/description.h"
#include "rabbits/dynloader/dynloader.h"
#include "rabbits/config/manager.h"

#include <cstring>

#include <boost/filesystem.hpp>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using boost::filesystem::path;
using std::string;

namespace test {
    extern const char * test_payload;
};

std::set<TestFactory*> * TestFactory::m_insts = NULL;

ComponentBase * TestBase::create_component_by_implem(const string implem,
                                               const string yml_params)
{
    ComponentManager &cm = get_config().get_component_manager();
    ComponentManager::Factory cf = cm.find_by_implem(implem);
    PlatformDescription descr;

    if (cf == NULL) {
        return NULL;
    }

    descr.load_yaml(yml_params);

    return cf->create(implem, descr);
}

static int do_test(TestFactory *tf, ConfigManager &config)
{

    TestBase *t = NULL;
    bool tests_passed;

    try {
        t = tf->create(config);
    } catch (TestFailureException e) {
        LOG(APP, ERR) << tf->get_name() << ": Failed during elaboration: " << e.what() << "\n";
        return 1;
    } catch (RabbitsException e) {
        LOG(APP, ERR) << tf->get_name() << ": Rabbits exception during elaboration.\n" << e.what() << "\n";
        return 1;
    } catch (sc_core::sc_report e) {
        LOG(APP, ERR) << tf->get_name() << ": SystemC report during elaboration.\n" << e.what() << "\n";
        return 1;
    } catch (std::exception e) {
        LOG(APP, ERR) << tf->get_name() << ": Unknown exception during elaboration.\n" << e.what() << "\n";
        return 1;
    } catch (...) {
        LOG(APP, ERR) << tf->get_name() << ": Unknown exception during elaboration.\n";
        return 1;
    }

    try {
        t->run();
    } catch (TestFailureException e) {
        LOG(APP, ERR) << tf->get_name() << ": Failed during test: " << e.what() << "\n";
        return 1;
    } catch (RabbitsException e) {
        LOG(APP, ERR) << tf->get_name() << ": Rabbits exception during test.\n" << e.what() << "\n";
        return 1;
    } catch (sc_core::sc_report e) {
        LOG(APP, ERR) << tf->get_name() << ": SystemC report during test.\n" << e.what() << "\n";
        return 1;
    } catch (std::exception e) {
        LOG(APP, ERR) << tf->get_name() << ": Unknown exception during test.\n" << e.what() << "\n";
        return 1;
    } catch (...) {
        LOG(APP, ERR) << tf->get_name() << ": Unknown exception during test.\n";
        return 1;
    }

    tests_passed = t->tests_passed();
    delete t;

    return (tests_passed) ? 0 : 1;
}

static int report_result(const string &name, int status)
{
    string res;
    int ret = 0;

    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status)) {
            res = "FAIL";
            ret = 1;
        } else {
            res = "OK";
            ret = 0;
        }
    } else if (WIFSIGNALED(status)) {
        res = "FAIL (" + string(strsignal(WTERMSIG(status))) + ")";
        ret = 1;
    }

    LOG(APP, INF) << name << "\r\t\t\t\t\t\t\t\t\t\t" << res << "\n";

    return ret;
}

std::string TestBase::get_test_dir(const std::string &fn) const
{
    return path(fn).parent_path().string();
}

static void load_test_module(DynamicLoader &dyn_loader)
{
    DynLib *tst = dyn_loader.load_library(test::test_payload);

    if (tst == NULL) {
        LOG(APP, ERR) << "Unable to load test module\n";
        exit(1);
    }
}

int sc_main(int argc, char *argv[])
{
    char *env_dynlib_paths;

    ConfigManager config;
    ConfigManager::set_config_manager(config);

    DynamicLoader &dyn_loader = config.get_dynloader();

    TestFactory::const_iterator it;
    int result = 0;

    if ((argc == 2) && (string(argv[1]) == "-d")) {
        get_app_logger().set_log_level(LogLevel::DEBUG);
        get_sim_logger().set_log_level(LogLevel::DEBUG);
    } else {
        get_app_logger().set_log_level(LogLevel::INFO);
        get_sim_logger().set_log_level(LogLevel::INFO);
    }

    env_dynlib_paths = std::getenv("RABBITS_DYNLIB_PATH");
    if (env_dynlib_paths != NULL) {
        dyn_loader.add_colon_sep_search_paths(env_dynlib_paths);
    }

    dyn_loader.search_and_load_rabbits_dynlibs();

    load_test_module(dyn_loader);

    for (it = TestFactory::begin(); it != TestFactory::end(); it++) {
        int pid = fork();
        int status;

        if (pid == -1) {
            perror("fork");
            return 2;
        }

        if (pid == 0) {
            return do_test(*it, config);
        }

        wait(&status);

        result |= report_result((*it)->get_name(), status);
    }

    return result;
}
