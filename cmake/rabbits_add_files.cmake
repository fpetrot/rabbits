# function to collect all the sources from sub-directories
# into a single list
function(rabbits_add_files_to_collection collection brief_doc full_doc)
    get_property(is_defined GLOBAL PROPERTY ${collection} DEFINED)
    if(NOT is_defined)
        define_property(GLOBAL PROPERTY ${collection}
            BRIEF_DOCS "${brief_doc}"
            FULL_DOCS "${full_doc}")
    endif()

    # make absolute paths
    set(SRCS)
    foreach(s IN LISTS ARGN)
        if(NOT IS_ABSOLUTE "${s}")
            get_filename_component(s "${s}" ABSOLUTE)
        endif()
        list(APPEND SRCS "${s}")
    endforeach()

    # append to global list
    set_property(GLOBAL APPEND PROPERTY ${collection} "${SRCS}")
endfunction(rabbits_add_files_to_collection)

function(rabbits_add_sources)
    rabbits_add_files_to_collection(
        RABBITS_SRCS_LIST
        "List of source files"
        "List of source files to be compiled in one library"
        ${ARGN})
endfunction(rabbits_add_sources)

function(rabbits_add_libs)
    get_property(is_defined GLOBAL PROPERTY RABBITS_LIBS_LIST DEFINED)
    if(NOT is_defined)
        define_property(GLOBAL PROPERTY RABBITS_SRCS_LIST
            BRIEF_DOCS "List of extra libraries"
            FULL_DOCS "List of extra libraries")
    endif()

    set_property(GLOBAL APPEND PROPERTY RABBITS_LIBS_LIST ${ARGN})
endfunction(rabbits_add_libs)

function(rabbits_add_components)
    rabbits_add_files_to_collection(
        RABBITS_COMPONENTS_LIST
        "List of component description files"
        "List of component description files"
        ${ARGN})
endfunction(rabbits_add_components)

function(rabbits_add_plugins)
    rabbits_add_files_to_collection(
        RABBITS_PLUGINS_LIST
        "List of plugins"
        "List of plugins with their header file and class name"
        ${ARGN})
endfunction(rabbits_add_plugins)

function(rabbits_add_platforms)
    rabbits_add_files_to_collection(
        RABBITS_PLATFORMS_LIST
        "List of platforms"
        "List of platform configuration files"
        ${ARGN})
    rabbits_add_files_to_collection(
        RABBITS_ALL_PLATFORMS_LIST
        "List of all platforms"
        "List of all platform configuration files"
        ${ARGN})
endfunction(rabbits_add_platforms)

function(rabbits_add_configs)
    rabbits_add_files_to_collection(
        RABBITS_CONFIGS_LIST
        "List of config files"
        "List of other configuration files"
        ${ARGN})
    rabbits_add_files_to_collection(
        RABBITS_ALL_CONFIGS_LIST
        "List of all config files"
        "List of all other configuration files"
        ${ARGN})
endfunction(rabbits_add_configs)

function(rabbits_add_res)
    rabbits_add_files_to_collection(
        RABBITS_RES_LIST
        "List of resources files"
        "List of resources files"
        ${ARGN})
    rabbits_add_files_to_collection(
        RABBITS_ALL_RES_LIST
        "List of all resources files"
        "List of all resources files"
        ${ARGN})
endfunction(rabbits_add_res)

function(rabbits_add_tests)
    rabbits_add_files_to_collection(
        RABBITS_TESTS_LIST
        "List of tests"
        "List of test source files"
        ${ARGN})
endfunction(rabbits_add_tests)

function(rabbits_add_executable n)
    set(RABBITS_DYNAMIC_PLUGIN OFF)
    rabbits_generate_objects(${n})
    get_property(__srcs GLOBAL PROPERTY RABBITS_SRCS_LIST)
    get_property(__libs GLOBAL PROPERTY RABBITS_LIBS_LIST)
    add_executable(${n} ${__srcs})
    target_link_libraries(${n} ${__libs})
    rabbits_generate_platform_symlinks(${n})
    rabbits_generate_tests(${n})
    rabbits_install_configs(${n})
    rabbits_install_res(${n})
    rabbits_clear_files_collections()
    install(TARGETS ${n} DESTINATION ${RABBITS_BIN_DIR})
endfunction(rabbits_add_executable)

function(rabbits_add_dynlib n)
    set(RABBITS_DYNAMIC_PLUGIN ON)
    rabbits_generate_objects(${n})
    get_property(__srcs GLOBAL PROPERTY RABBITS_SRCS_LIST)
    get_property(__libs GLOBAL PROPERTY RABBITS_LIBS_LIST)
    add_library(${n} SHARED ${__srcs})
    target_link_libraries(${n} ${__libs})
    set_property(TARGET ${n} PROPERTY PREFIX "")
    rabbits_generate_platform_symlinks(${n})
    rabbits_generate_tests(${n})
    rabbits_install_configs(${n})
    rabbits_install_res(${n})
    rabbits_clear_files_collections()
    install(TARGETS ${n} DESTINATION ${RABBITS_LIB_DIR}/rabbits)
endfunction(rabbits_add_dynlib)

function(rabbits_add_static_library n)
    set(RABBITS_DYNAMIC_PLUGIN OFF)
    set(do_export FALSE)

    foreach(opt IN LISTS ARGN)
        if (${opt} STREQUAL EXPORT)
            set(do_export TRUE)
        endif()
    endforeach()

    rabbits_generate_objects(${n})
    get_property(__srcs GLOBAL PROPERTY RABBITS_SRCS_LIST)
    get_property(__libs GLOBAL PROPERTY RABBITS_LIBS_LIST)
    add_library(${n} STATIC ${__srcs})
    target_link_libraries(${n} ${__libs} ${RABBITS_LIBRARIES})
    rabbits_generate_platform_symlinks(${n})
    rabbits_generate_tests(${n})
    rabbits_install_configs(${n})
    rabbits_install_res(${n})
    rabbits_clear_files_collections()

    if(do_export)
        install(TARGETS ${n} EXPORT ${n} DESTINATION ${RABBITS_LIB_DIR})
        install(EXPORT ${n} DESTINATION ${RABBITS_LIB_DIR}/${n})
    else()
        install(TARGETS ${n} DESTINATION ${RABBITS_LIB_DIR})
    endif()

endfunction(rabbits_add_static_library)

function(rabbits_clear_files_collection)
    foreach(c IN LISTS ARGN)
        set_property(GLOBAL PROPERTY ${c})
    endforeach()
endfunction(rabbits_clear_files_collection)

function(rabbits_clear_files_collections)
    rabbits_clear_files_collection(
        RABBITS_SRCS_LIST
        RABBITS_LIBS_LIST
        RABBITS_COMPONENTS_LIST
        RABBITS_PLUGINS_LIST
        RABBITS_PLATFORMS_LIST
        RABBITS_CONFIGS_LIST
        RABBITS_RES_LIST
        RABBITS_TESTS_LIST)
endfunction(rabbits_clear_files_collections)

function(rabbits_debian_package)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Rabbits auto-packaging")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "rabbits")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Auto-generated Rabbits package for project ${CMAKE_PROJECT_NAME}")

    rabbits_get_version(v)
    set(CPACK_PACKAGE_VERSION ${v})
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)

    include(CPack)
endfunction(rabbits_debian_package)

# vim: ts=4 sts=4 sw=4 expandtab
