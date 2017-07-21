cmake_minimum_required (VERSION 3.0)

if("${CMAKE_VERSION}" VERSION_LESS 3.1)
    # XXX Not portable
    add_compile_options(-std=c++11)
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(RABBITS_FACTORY_SCRIPT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-factory.rb")
set(RABBITS_LIST_PLATFORMS_SCRIPT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts/list-platforms.rb")
include(rabbits_add_files)
include(rabbits_components)

rabbits_get_version(RABBITS_VERSION)

project (Rabbits VERSION ${RABBITS_VERSION})

find_package(Threads REQUIRED)
find_package(SystemC REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(RubyInterp 2.0 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(Doxygen)
find_package(libfdt REQUIRED)

# Options
option(USE_QT "Use QT for user interface" ON)
option(BUILD_DOCUMENTATION "Build the Doxygen HTML documentation (requires Doxygen)" OFF)
option(RABBITS_ENABLE_TESTING "Enable tests building" OFF)
option(BUILD_RABBITS_EXECUTABLE "Build the rabbits executable. If off, only the library is built" ON)

set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${LIBFDT_LIBRARIES})

if(USE_QT)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOMOC ON)
    find_package(Qt5Widgets)
    find_package(Qt5WebKitWidgets)
    set(RABBITS_CONFIG_QT 1)
    set(EXTRA_LIBS ${EXTRA_LIBS} Qt5::Widgets Qt5::WebKitWidgets)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT RABBITS_LOGLEVEL)
        set(RABBITS_LOGLEVEL 4) # Trace level
    endif()
    set(RABBITS_DEBUG ON)
else()
    if(NOT RABBITS_LOGLEVEL)
        set(RABBITS_LOGLEVEL 4) # Trace level also
    endif()
endif()

if(RABBITS_ENABLE_TESTING)
    enable_testing()
endif()

if(UNIX)
    set(RABBITS_CONFIG_POSIX 1)
    option(RABBITS_CREATE_PLATFORM_SYMLINK
        "Create one symlink per platform pointing on rabbits executable"
        ON)
    if(NOT RABBITS_PLATFORM_SYMLINK_PREFIX)
        set(RABBITS_PLATFORM_SYMLINK_PREFIX "rabbits-")
    endif()
elseif(WIN32)
    set(RABBITS_CONFIG_WIN32 1)
else()
    message(FATAL_ERROR, "Unsupported operating system")
endif()

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
    "${SYSTEMC_INCLUDE_DIR}"
    "${YAMLCPP_INCLUDE_DIR}"
    "${Boost_INCLUDE_DIR}"
)

set(EXTRA_LIBS ${EXTRA_LIBS}
    ${SYSTEMC_LIBRARIES}
    ${YAMLCPP_LIBRARY}
    ${Boost_LIBRARIES}
)

set(RABBITS_DYNLIB_SEARCH_PATH "${CMAKE_INSTALL_PREFIX}/lib/rabbits")
set(RABBITS_APP_NAME rabbits)

# By convention, API Version is a combination of Rabbits major and minor version.
math(EXPR RABBITS_API_VERSION "${PROJECT_VERSION_MAJOR} * 100 + ${PROJECT_VERSION_MINOR}")

if(NOT RABBITS_API_VERSION MATCHES "^[0-9]+$")
    # fallback
    set(RABBITS_API_VERSION 0)
    message(WARNING "Rabbits version number is invalid.")
endif()

set(RABBITS_BIN_DIR bin)
set(RABBITS_LIB_DIR lib)
set(RABBITS_INCLUDE_DIR include)
set(RABBITS_DOC_DIR share/doc/rabbits)
set(RABBITS_CONFIG_DIR share/rabbits/config)
set(RABBITS_RES_DIR share/rabbits/res)
set(RABBITS_EXECUTABLE ${RABBITS_BIN_DIR}/${RABBITS_APP_NAME})
set(RABBITS_LIBRARIES rabbits)
set(RABBITS_TEST_LIBRARY rabbits-test)

# config.h generation
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/rabbits/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/rabbits/config.h"
)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")


add_subdirectory(librabbits)

if (BUILD_RABBITS_EXECUTABLE)
    add_subdirectory(rabbits)
endif()

if(BUILD_DOCUMENTATION AND NOT DOXYGEN_FOUND)
    message(WARNING "Doxygen was not found, skipping documentation building.")
endif()

if(DOXYGEN_FOUND)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY)

    unset(_all)
    if(BUILD_DOCUMENTATION)
        set(_all ALL)
    endif()

    add_custom_target(doc ${_all}
        COMMAND ${DOXYGEN_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    if(BUILD_DOCUMENTATION)
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html
            DESTINATION ${RABBITS_DOC_DIR})
    endif()
endif()

# RabbitsConfig[Version].cmake generation
include(CMakePackageConfigHelpers)
configure_package_config_file(RabbitsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/RabbitsConfig.cmake
    INSTALL_DESTINATION ${RABBITS_LIB_DIR}/rabbits/cmake
    PATH_VARS RABBITS_INCLUDE_DIR
    RABBITS_LIB_DIR
    RABBITS_BIN_DIR
    RABBITS_CONFIG_DIR
    RABBITS_RES_DIR
    RABBITS_EXECUTABLE
    NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/RabbitsConfigVersion.cmake
    VERSION ${RABBITS_VERSION}
    COMPATIBILITY SameMajorVersion)

install(EXPORT rabbits-targets DESTINATION ${RABBITS_LIB_DIR}/rabbits/cmake)

# header files and scripts installation
install(DIRECTORY include/ DESTINATION ${RABBITS_INCLUDE_DIR} FILES_MATCHING PATTERN "*.h")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/rabbits/config.h" DESTINATION ${RABBITS_INCLUDE_DIR}/rabbits)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-factory.rb" DESTINATION ${RABBITS_LIB_DIR}/rabbits)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/scripts/list-platforms.rb" DESTINATION ${RABBITS_LIB_DIR}/rabbits)
install(DIRECTORY DESTINATION ${RABBITS_CONFIG_DIR})
install(DIRECTORY DESTINATION ${RABBITS_RES_DIR})

# cmake files installation
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RabbitsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/RabbitsConfigVersion.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/rabbits_add_files.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/rabbits_components.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindSystemC.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindRubyInterp.cmake
    DESTINATION ${RABBITS_LIB_DIR}/rabbits/cmake)


# for sub-projects build without rabbits install
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/RabbitsConfig_installless.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/installless/RabbitsConfig.cmake"
    @ONLY
)
set(Rabbits_DIR "${CMAKE_CURRENT_BINARY_DIR}/installless" CACHE INTERNAL "RabbitsConfig.cmake directory" FORCE)

# vim: ts=4 sts=4 sw=4 expandtab
