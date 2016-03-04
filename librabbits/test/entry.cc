#include "rabbits/test/test.h"
#include "rabbits/logger.h"
#include "rabbits/component/manager.h"
#include "rabbits/component/factory.h"
#include "rabbits/platform/description.h"
#include "rabbits/dynloader/dynloader.h"

#include <cstring>

#include <boost/filesystem.hpp>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using boost::filesystem::path;
using std::string;

std::vector<TestFactory*> * TestFactory::m_insts = NULL;

ComponentBase * Test::create_component_by_name(const string name,
                                               const string yml_params)
{
    ComponentManager &cm = ComponentManager::get();
    ComponentFactory *cf = cm.find_by_name(name);
    PlatformDescription descr;

    if (cf == NULL) {
        return NULL;
    }

    descr.load_yaml(yml_params);

    return cf->create(name, descr);
}

static int do_test(TestFactory *tf)
{

    Test *t = NULL;
    bool tests_passed;

    try {
        t = tf->create();
    } catch (TestFailureException e) {
        ERR_STREAM(tf->get_name() << ": Failed during elaboration: " << e.what_without_bt() << "\n");
        return 1;
    } catch (sc_core::sc_report e) {
        ERR_STREAM(tf->get_name() << ": SystemC report during elaboration.\n" << e.what() << "\n");
        return 1;
    } catch (std::exception e) {
        ERR_STREAM(tf->get_name() << ": Unknown exception during elaboration.\n" << e.what() << "\n");
        return 1;
    } catch (...) {
        ERR_STREAM(tf->get_name() << ": Unknown exception during elaboration.\n");
        return 1;
    }

    try {
        sc_core::sc_start();
    } catch (TestFailureException e) {
        ERR_STREAM(tf->get_name() << ": Failed during test: " << e.what_without_bt() << "\n");
        return 1;
    } catch (sc_core::sc_report e) {
        ERR_STREAM(tf->get_name() << ": SystemC report during test.\n" << e.what() << "\n");
        return 1;
    } catch (std::exception e) {
        ERR_STREAM(tf->get_name() << ": Unknown exception during test.\n" << e.what() << "\n");
        return 1;
    } catch (...) {
        ERR_STREAM(tf->get_name() << ": Unknown exception during test.\n");
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

    INF_STREAM(name << "\r\t\t\t\t\t\t" << res << "\n");

    return ret;
}

std::string Test::get_test_dir(const std::string &fn) const
{
    return path(fn).parent_path().string();
}

int sc_main(int argc, char *argv[])
{
    char *env_dynlib_paths;
    DynamicLoader &dyn_loader = DynamicLoader::get();
    TestFactory::const_iterator it;
    int result = 0;

    if ((argc == 2) && (string(argv[1]) == "-d")) {
        Logger::get().set_log_level(LogLevel::DEBUG);
    } else {
        Logger::get().set_log_level(LogLevel::INFO);
    }

    env_dynlib_paths = std::getenv("RABBITS_DYNLIB_PATH");
    if (env_dynlib_paths != NULL) {
        dyn_loader.add_colon_sep_search_paths(env_dynlib_paths);
    }

    dyn_loader.search_and_load_rabbits_dynlibs();
    ComponentManager::get(); /* Force instantiation of ComponentManager
                                and registration of components before forking */

    for (it = TestFactory::begin(); it != TestFactory::end(); it++) {
        int pid = fork();
        int status;

        if (pid == -1) {
            perror("fork");
            return 2;
        }

        if (pid == 0) {
            return do_test(*it);
        }

        wait(&status);

        result |= report_result((*it)->get_name(), status);
    }

    return result;
}
