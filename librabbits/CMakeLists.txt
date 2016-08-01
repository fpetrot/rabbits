include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(channel)
add_subdirectory(component)
add_subdirectory(config)
add_subdirectory(datatypes)
add_subdirectory(loader)
add_subdirectory(logger)
add_subdirectory(module)
add_subdirectory(platform)
add_subdirectory(ui)
add_subdirectory(dynloader)
add_subdirectory(../components ../components)
add_subdirectory(../backends ../backends)
add_subdirectory(../plugins ../plugins)
rabbits_add_sources(
	simu.cc
	exception.cc
)


rabbits_generate_objects("librabbits")

get_property(srcs GLOBAL PROPERTY RABBITS_SRCS_LIST)
get_property(libs GLOBAL PROPERTY RABBITS_LIBS_LIST)
add_library(${RABBITS_LIBRARIES} ${srcs})
target_link_libraries(${RABBITS_LIBRARIES} ${EXTRA_LIBS} ${libs})

install(TARGETS ${RABBITS_LIBRARIES} DESTINATION lib EXPORT rabbits-targets)

rabbits_clear_files_collections()


add_subdirectory(test)

get_property(srcs GLOBAL PROPERTY RABBITS_SRCS_LIST)
get_property(libs GLOBAL PROPERTY RABBITS_LIBS_LIST)
add_library(${RABBITS_TEST_LIBRARY} ${srcs})
target_link_libraries(${RABBITS_TEST_LIBRARY} -Wl,--whole-archive ${RABBITS_LIBRARIES} -Wl,--no-whole-archive)

install(TARGETS ${RABBITS_TEST_LIBRARY} DESTINATION lib EXPORT rabbits-targets)

rabbits_clear_files_collections()
