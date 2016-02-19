#include "rabbits/test/test.h"
#include "rabbits/logger.h"
#include "rabbits/component/manager.h"
#include "rabbits/component/factory.h"
#include "rabbits/platform/description.h"
#include "rabbits/dynloader/dynloader.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


std::vector<TestFactory*> * TestFactory::m_insts = NULL;

ComponentBase * Test::create_component_by_name(const std::string name,
                                               const std::string yml_params)
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
    try {
        Test *t = tf->create();
        bool tests_passed;

        sc_core::sc_start();

        tests_passed = t->tests_passed();
        delete t;

        return (tests_passed) ? 0 : 1;

    } catch (TestFailureException e) {
        ERR_STREAM(tf->get_name() << ": Failed during elaboration: " << e.what_without_bt() << "\n");
        return 1;
    } catch (std::runtime_error e) {
        ERR_STREAM(tf->get_name() << ": Unknown exception during elaboration.\n" << e.what() << "\n");
        return 1;
    } catch (...) {
        ERR_STREAM(tf->get_name() << ": Unknown exception during elaboration.\n");
        return 1;
    }
}

int sc_main(int argc, char *argv[])
{
    char *env_dynlib_paths;
    DynamicLoader &dyn_loader = DynamicLoader::get();
    TestFactory::const_iterator it;
    int result = 0;

    Logger::get().set_log_level(LogLevel::INFO);

    env_dynlib_paths = std::getenv("RABBITS_DYNLIB_PATH");
    if (env_dynlib_paths != NULL) {
        dyn_loader.add_semicol_sep_search_paths(env_dynlib_paths);
    }

    dyn_loader.search_and_load_rabbits_dynlibs();

    for (it = TestFactory::begin(); it != TestFactory::end(); it++) {
        int pid = fork();
        int status;
        int test_result;

        if (pid == -1) {
            perror("fork");
            return 2;
        }

        if (pid == 0) {
            return do_test(*it);
        }

        wait(&status);

        if (WIFEXITED(status)) {
            test_result = WEXITSTATUS(status);
        } else {
            test_result = 1;
        }

        INF_STREAM((*it)->get_name() << "\r\t\t\t\t\t\t" << ((test_result == 0) ? "OK" : "FAIL") << "\n");

        result |= test_result;
    }

    return result;
}
