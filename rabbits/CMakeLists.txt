rabbits_add_sources(
    rabbits.cc
    usage.cc
)

get_property(srcs GLOBAL PROPERTY RABBITS_SRCS_LIST)
get_property(libs GLOBAL PROPERTY RABBITS_LIBS_LIST)
add_executable(rabbits-bin ${srcs})
set_property(TARGET rabbits-bin PROPERTY OUTPUT_NAME ${RABBITS_APP_NAME})

target_link_libraries(rabbits-bin
    ${SYSTEMC_LIBRARIES}
    ${libs}
    -Wl,--whole-archive rabbits -Wl,--no-whole-archive
)

install(TARGETS rabbits-bin DESTINATION ${RABBITS_BIN_DIR})

rabbits_clear_files_collections()

# vim: ts=4 sts=4 sw=4 expandtab
