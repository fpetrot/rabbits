function(rabbits_generate_objects)
    if(${Rabbits_FOUND})
        set(_gen_script "${Rabbits_LIB_DIR}/rabbits/gen-factory.rb")
    else()
        set(_gen_script "${PROJECT_SOURCE_DIR}/scripts/gen-factory.rb")
    endif()
    set(_gen_exe ${RUBY_EXECUTABLE} "${_gen_script}")

    get_property(COMPS_DESCR GLOBAL PROPERTY RABBITS_COMPONENTS_LIST)
    get_property(PLUGIN_DESCR GLOBAL PROPERTY RABBITS_PLUGINS_LIST)

    set(_objs ${COMPS_DESCR} ${PLUGIN_DESCR})

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

    add_custom_command(
        OUTPUT "${_out}"
        COMMAND ${_gen_exe} ${_gen_arg} -s "${PROJECT_SOURCE_DIR}" -b "${PROJECT_BINARY_DIR}" -o "${_out}" ${_objs}
        DEPENDS ${_objs} ${_gen_script})

    set_property(GLOBAL APPEND PROPERTY RABBITS_SRCS_LIST "${_out}")
endfunction(rabbits_generate_objects)

# vim: ts=4 sts=4 sw=4 expandtab
