rabbits_clear_files_collections()

rabbits_add_sources(rabbits.cc)

get_property(RABBITS_SRCS GLOBAL PROPERTY RABBITS_SRCS_LIST)
add_executable(rabbits ${RABBITS_SRCS})
target_link_libraries(rabbits ${SYSTEMC_LIBRARIES} -Wl,--whole-archive librabbits -Wl,--no-whole-archive)

install(TARGETS rabbits DESTINATION bin)
