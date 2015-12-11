# function to collect all the sources from sub-directories
# into a single list
function(rabbits_add_files_to_collection collection brief_doc full_doc)
    get_property(is_defined GLOBAL PROPERTY ${collection} DEFINED)
    if(NOT is_defined)
        define_property(GLOBAL PROPERTY RABBITS_SRCS_LIST
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
        "List of platform description files"
        ${ARGN})
endfunction(rabbits_add_platforms)

function(rabbits_add_executable n)
    set(RABBITS_DYNAMIC_PLUGIN OFF)
    rabbits_generate_objects()
    get_property(__srcs GLOBAL PROPERTY RABBITS_SRCS_LIST)
    get_property(__libs GLOBAL PROPERTY RABBITS_LIBS_LIST)
    add_executable(${n} ${__srcs})
    target_link_libraries(${n} ${__libs})
    rabbits_generate_descr_symlinks()
    rabbits_clear_files_collections()
    install(TARGETS ${n} DESTINATION ${RABBITS_BIN_DIR})
endfunction(rabbits_add_executable)

function(rabbits_add_dynlib n)
    set(RABBITS_DYNAMIC_PLUGIN ON)
    rabbits_generate_objects()
    get_property(__srcs GLOBAL PROPERTY RABBITS_SRCS_LIST)
    get_property(__libs GLOBAL PROPERTY RABBITS_LIBS_LIST)
    add_library(${n} SHARED ${__srcs})
    target_link_libraries(${n} ${__libs})
    set_property(TARGET ${n} PROPERTY PREFIX "")
    rabbits_generate_descr_symlinks()
    rabbits_clear_files_collections()
    install(TARGETS ${n} DESTINATION ${RABBITS_LIB_DIR}/rabbits)
endfunction(rabbits_add_dynlib)

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
        RABBITS_PLATFORMS_LIST)
endfunction(rabbits_clear_files_collections)

# vim: ts=4 sts=4 sw=4 expandtab
