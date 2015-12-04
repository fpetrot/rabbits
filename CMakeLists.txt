cmake_minimum_required (VERSION 2.6)
project (Rabbits)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(rabbits_add_files)
include(rabbits_components)

find_package(SystemC REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Ruby 2.0 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

# Options
option(USE_SDL "Use SDL1.2 for framebuffer devices" ON)
option(RABBITS_DESCR_SEARCH_PATH 
	"Search path for description files" 
	"${CMAKE_INSTALL_PREFIX}/share/rabbits/descr")

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
set(RABBITS_APP_NAME "rabbits")

# XXX
set(RABBITS_VERSION 0.0.1)
set(LIB_INSTALL_DIR lib)
set(INCLUDE_INSTALL_DIR include)

include(compiler_options)

# config.h generation
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/include/rabbits/config.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/include/rabbits/config.h"
)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")

# ConfigRabbits.cmake generation
include(CMakePackageConfigHelpers)
configure_package_config_file(RabbitsConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/RabbitsConfig.cmake
	INSTALL_DESTINATION ${LIB_INSTALL_DIR}/rabbits/cmake
	PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR)
write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/RabbitsConfigVersion.cmake
	VERSION ${RABBITS_VERSION}
	COMPATIBILITY SameMajorVersion )


add_subdirectory(librabbits)
add_subdirectory(rabbits)


# header files and scripts installation
install(DIRECTORY include/ DESTINATION ${INCLUDE_INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/rabbits/config.h" DESTINATION ${INCLUDE_INSTALL_DIR}/rabbits)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-factory.rb" DESTINATION ${LIB_INSTALL_DIR}/rabbits)


# cmake files installation
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RabbitsConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/RabbitsConfigVersion.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/rabbits_add_files.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/rabbits_components.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindSystemC.cmake
	DESTINATION ${LIB_INSTALL_DIR}/rabbits/cmake )
