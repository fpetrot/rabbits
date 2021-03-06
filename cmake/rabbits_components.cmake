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
            set(_out ${CMAKE_CURRENT_BINARY_DIR}/static_loader.cc)
            set(_gen_arg "--static-loader")
        endif()

        rabbits_get_version(v)

        unset(_ver)
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

function(rabbits_generate_platform_symlinks n)
    if(NOT RABBITS_CREATE_PLATFORM_SYMLINK)
        return()
    endif()

    set(_list_script ${RABBITS_LIST_PLATFORMS_SCRIPT_PATH})
    set(_list_exe ${RUBY_EXECUTABLE} "${_list_script}")

    get_property(__configs GLOBAL PROPERTY RABBITS_CONFIGS_LIST)

    execute_process(
        COMMAND ${_list_exe} ${__configs}
        OUTPUT_VARIABLE __platforms)

    foreach(p IN LISTS __platforms)
        get_filename_component(p_name "${p}" NAME_WE)
        get_filename_component(rabbits_app_name "${RABBITS_EXECUTABLE}" NAME)

        set(__sym ${CMAKE_CURRENT_BINARY_DIR}/${RABBITS_PLATFORM_SYMLINK_PREFIX}${p_name})
        add_custom_target(
            ${p_name}_symlink
            ALL
            COMMAND ln -sf "${rabbits_app_name}" "${__sym}"
        )

        install(FILES ${__sym} DESTINATION ${RABBITS_BIN_DIR})
    endforeach()
endfunction(rabbits_generate_platform_symlinks)

function(rabbits_generate_tests n)
    if (NOT RABBITS_ENABLE_TESTING)
        return()
    endif()

    get_property(_tests GLOBAL PROPERTY RABBITS_TESTS_LIST)
    set(_test_name ${n}_test)

    if(_tests)
        set(_test_mod ${_test_name}_mod)
        add_library(${_test_mod} MODULE ${_tests})

        set(_test_payload ${CMAKE_CURRENT_BINARY_DIR}/${n}_test_payload.cc)

        add_custom_command(
            OUTPUT "${_test_payload}"
            COMMAND ${CMAKE_COMMAND} -E echo
                'namespace test { const char * test_payload = \"$<TARGET_FILE:${_test_mod}>\"\; }\;'
                >"${_test_payload}")

        add_executable(${_test_name} ${_test_payload})
        target_link_libraries(${_test_name} ${SYSTEMC_LIBRARIES} ${RABBITS_TEST_LIBRARY})
        add_test(NAME ${_test_name} COMMAND ${_test_name})
    endif()
endfunction(rabbits_generate_tests)

function(rabbits_install_configs n)
    get_property(__configs GLOBAL PROPERTY RABBITS_CONFIGS_LIST)
    if(__configs)
        install(FILES ${__configs} DESTINATION ${RABBITS_CONFIG_DIR}/${n}/config)
    endif()
endfunction(rabbits_install_configs)

function(rabbits_install_res n)
    get_property(__res GLOBAL PROPERTY RABBITS_RES_LIST)

    foreach (r IN LISTS __res)
        set(inv_prop "RABBITS_RES_MAP_INV_${r}")
        set(dest_prop "RABBITS_RES_MAP_DEST_${r}")

        get_property(inv GLOBAL PROPERTY "${inv_prop}")
        get_property(dest GLOBAL PROPERTY "${dest_prop}")

        set(final_dest "${RABBITS_RES_DIR}/${inv}/${dest}")
        get_filename_component(final_dir ${final_dest} DIRECTORY)
        get_filename_component(final_name ${final_dest} NAME)

        install(FILES "${r}" DESTINATION "${final_dir}" RENAME "${final_name}")
    endforeach()
endfunction(rabbits_install_res)

# vim: ts=4 sts=4 sw=4 expandtab
