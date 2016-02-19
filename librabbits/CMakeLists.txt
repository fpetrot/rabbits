include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(channel)
add_subdirectory(component)
add_subdirectory(loader)
add_subdirectory(platform)
add_subdirectory(plugin)
add_subdirectory(ui)
add_subdirectory(dynloader)
add_subdirectory(../components ../components)
add_subdirectory(../plugins ../plugins)
rabbits_add_sources(
	logger.cc
	simu.cc
	exception.cc
)


rabbits_generate_objects()

get_property(srcs GLOBAL PROPERTY RABBITS_SRCS_LIST)
get_property(libs GLOBAL PROPERTY RABBITS_LIBS_LIST)
add_library(librabbits ${srcs})
target_link_libraries(librabbits ${EXTRA_LIBS} ${libs})
set_property(TARGET librabbits PROPERTY OUTPUT_NAME rabbits)

install(TARGETS librabbits DESTINATION lib)

rabbits_clear_files_collections()


add_subdirectory(test)

get_property(srcs GLOBAL PROPERTY RABBITS_SRCS_LIST)
get_property(libs GLOBAL PROPERTY RABBITS_LIBS_LIST)
add_library(librabbits-test ${srcs})
target_link_libraries(librabbits-test -Wl,--whole-archive librabbits -Wl,--no-whole-archive)
set_property(TARGET librabbits-test PROPERTY OUTPUT_NAME rabbits-test)

rabbits_clear_files_collections()
