function(rabbits_get_version_from_file v)
    file(READ ${RABBITS_VERSION_FILE} _v)
    string(STRIP ${_v} _v)
    set(${v} ${_v} PARENT_SCOPE)
endfunction(rabbits_get_version_from_file)

function(rabbits_get_version v)
    set(RABBITS_VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/VERSION PARENT_SCOPE)
    set(RABBITS_VERSION_FILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/VERSION PARENT_SCOPE)
    set(RABBITS_VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/VERSION)
    set(RABBITS_VERSION_FILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/VERSION)

    if(EXISTS ${RABBITS_VERSION_FILE})
        configure_file(
            ${RABBITS_VERSION_FILE}
            ${RABBITS_VERSION_FILE_OUT}
            COPYONLY)

        rabbits_get_version_from_file(_v)
    else()
        set(_v "0.0.1")
    endif()

    set(${v} ${_v} PARENT_SCOPE)
endfunction(rabbits_get_version)

function(rabbits_generate_objects n)
    set(_gen_script ${RABBITS_FACTORY_SCRIPT_PATH})
    set(_gen_exe ${RUBY_EXECUTABLE} "${_gen_script}")

    get_property(COMPS_DESCR GLOBAL PROPERTY RABBITS_COMPONENTS_LIST)
    get_property(PLUGIN_DESCR GLOBAL PROPERTY RABBITS_PLUGINS_LIST)

    set(_objs ${COMPS_DESCR} ${PLUGIN_DESCR})

    if(_objs)
        foreach(c IN LISTS _objs)
            get_filename_component(_c_src_dir "${c}" DIRECTORY)
            get_filename_component(_c_name "${c}" NAME)

            string(REPLACE "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" _c_bin_dir "${_c_src_dir}")
            set(_out "${_c_bin_dir}/${_c_name}.h")

            add_custom_command(
                OUTPUT ${_out}
                COMMAND ${_gen_exe} --factory -o "${_out}" "${c}"
                DEPENDS ${c} ${_gen_script})

            set_property(GLOBAL APPEND PROPERTY RABBITS_SRCS_LIST "${_out}")
        endforeach()

        if(RABBITS_DYNAMIC_PLUGIN)
            set(_out ${CMAKE_CURRENT_BINARY_DIR}/dyn_loader.cc)
            set(_gen_arg "--dyn-loader")
        else()
            set(_out ${CMAKE_CURRENT_BINARY_DIR}/static_insts.cc)
            set(_gen_arg "--static-insts")
        endif()

        rabbits_get_version(v)

        if(EXISTS ${RABBITS_VERSION_FILE_OUT})
            set(_ver ${RABBITS_VERSION_FILE_OUT})
        endif()

        add_custom_command(
            OUTPUT "${_out}"
            COMMAND ${_gen_exe} ${_gen_arg} -s "${PROJECT_SOURCE_DIR}" -b "${PROJECT_BINARY_DIR}" -m "${n}" -v "${v}" -o "${_out}" ${_objs}
            DEPENDS ${_objs} ${_gen_script} ${_ver})

        set_property(GLOBAL APPEND PROPERTY RABBITS_SRCS_LIST "${_out}")
    endif()

endfunction(rabbits_generate_objects)

function(rabbits_generate_descr_symlinks)
    get_property(PLATFORM_DESCR GLOBAL PROPERTY RABBITS_PLATFORMS_LIST)

    foreach(p IN LISTS PLATFORM_DESCR)
        install(FILES ${p} DESTINATION ${RABBITS_DESCRIPTION_DIR})

        if(RABBITS_CREATE_DESCR_SYMLINK)
            get_filename_component(p_name "${p}" NAME_WE)
            get_filename_component(rabbits_app_name "${RABBITS_EXECUTABLE}" NAME)

            set(__sym ${RABBITS_BIN_DIR}/${RABBITS_DESCR_SYMLINK_PREFIX}${p_name})

            install(CODE "EXECUTE_PROCESS(COMMAND ln -sf \"${rabbits_app_name}\" \"${__sym}\")")
        endif()
    endforeach()
endfunction(rabbits_generate_descr_symlinks)

function(rabbits_generate_tests n)
    get_property(_tests GLOBAL PROPERTY RABBITS_TESTS_LIST)
    string(CONCAT _test_name ${n} "_test")

    if(_tests)
        add_executable(${_test_name} ${_tests})
        target_link_libraries(${_test_name} librabbits-test ${n})
        add_test(NAME ${_test_name} COMMAND ${_test_name})
    endif()
endfunction(rabbits_generate_tests)

# vim: ts=4 sts=4 sw=4 expandtab
