cmake_minimum_required (VERSION 2.6)
project (RABBITS)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(RABBITS_FACTORY_SCRIPT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-factory.rb")
include(rabbits_add_files)
include(rabbits_components)

find_package(SystemC REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Ruby 2.0 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

# Options
option(USE_SDL "Use SDL1.2 for framebuffer devices" ON)

if(NOT RABBITS_DESCR_SEARCH_PATH)
    set(RABBITS_DESCR_SEARCH_PATH "${CMAKE_INSTALL_PREFIX}/share/rabbits/descr")
endif()

if(USE_SDL)
	find_package(SDL)
	if(SDL_FOUND)
		set(RABBITS_CONFIG_SDL 1)
		include_directories(${SDL_INCLUDE_DIR})
		set(EXTRA_LIBS ${EXTRA_LIBS} ${SDL_LIBRARY})
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	if(NOT RABBITS_LOGLEVEL)
		set(RABBITS_LOGLEVEL 3) # Debug level
	endif()
	set(RABBITS_DEBUG ON)
else()
	if(NOT RABBITS_LOGLEVEL)
		set(RABBITS_LOGLEVEL 1) # Errors and warnings
	endif()
endif()

if(UNIX)
	set(RABBITS_CONFIG_POSIX 1)
	option(RABBITS_CREATE_DESCR_SYMLINK
		"Create one symlink per platform description pointing on rabbits executable"
		ON)
	if(NOT RABBITS_DESCR_SYMLINK_PREFIX)
		set(RABBITS_DESCR_SYMLINK_PREFIX "rabbits-")
	endif()
elseif(WIN32)
	set(RABBITS_CONFIG_WIN32 1)
else()
	message(FATAL_ERROR, "Unsupported operating system")
endif()

include_directories(
	"${PROJECT_SOURCE_DIR}"
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

rabbits_get_version(RABBITS_VERSION)

set(RABBITS_BIN_DIR bin)
set(RABBITS_LIB_DIR lib)
set(RABBITS_INCLUDE_DIR include)
set(RABBITS_DESCRIPTION_DIR share/rabbits/platforms)
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
add_subdirectory(rabbits)

# RabbitsConfig[Version].cmake generation
include(CMakePackageConfigHelpers)
configure_package_config_file(RabbitsConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/RabbitsConfig.cmake
	INSTALL_DESTINATION ${RABBITS_LIB_DIR}/rabbits/cmake
	PATH_VARS RABBITS_INCLUDE_DIR
		  RABBITS_LIB_DIR
		  RABBITS_BIN_DIR
		  RABBITS_DESCRIPTION_DIR
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
install(DIRECTORY DESTINATION ${RABBITS_DESCRIPTION_DIR})

# cmake files installation
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RabbitsConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/RabbitsConfigVersion.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/rabbits_add_files.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/rabbits_components.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindSystemC.cmake
	DESTINATION ${RABBITS_LIB_DIR}/rabbits/cmake)


# for sub-projects build without rabbits install
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/RabbitsConfig_installless.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/installless/RabbitsConfig.cmake"
	@ONLY
)
set(Rabbits_DIR "${CMAKE_CURRENT_BINARY_DIR}/installless" CACHE INTERNAL "RabbitsConfig.cmake directory" FORCE)

# vim: ts=4 sts=4 sw=4 expandtab
