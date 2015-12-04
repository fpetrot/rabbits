rabbits_clear_files_collections()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

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

get_property(RABBITS_SRCS GLOBAL PROPERTY RABBITS_SRCS_LIST)
get_property(RABBITS_LIBS GLOBAL PROPERTY RABBITS_LIBS_LIST)
add_library(librabbits ${RABBITS_SRCS})
target_link_libraries(librabbits ${EXTRA_LIBS} ${RABBITS_LIBS})
set_property(TARGET librabbits PROPERTY OUTPUT_NAME rabbits)

install(TARGETS librabbits DESTINATION lib)
