rabbits_add_sources(rabbits.cc)

get_property(srcs GLOBAL PROPERTY RABBITS_SRCS_LIST)
get_property(libs GLOBAL PROPERTY RABBITS_LIBS_LIST)
add_executable(${RABBITS_APP_NAME} ${srcs})
target_link_libraries(${RABBITS_APP_NAME}
    ${SYSTEMC_LIBRARIES}
    ${libs}
    -Wl,--whole-archive librabbits -Wl,--no-whole-archive
)

install(TARGETS rabbits DESTINATION ${RABBITS_BIN_DIR})

rabbits_clear_files_collections()

# vim: ts=4 sts=4 sw=4 expandtab
