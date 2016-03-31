# - Find Ruby Interpreter
# This module finds if Ruby is installed and determines where the intepreter
# is.
#
# The minimum required version of Ruby can be specified using the
# standard syntax, e.g. find_package(RubyInterp 1.8)
#
# It also determines what the name of the library is. This
# code sets the following variables:
#
#  RUBY_EXECUTABLE   = full path to the ruby binary
#  RUBY_VERSION      = the version of ruby which was found, e.g. "1.8.7"
#  RUBYINTERP_FOUND  = set to true if ruby was found successfully

#=============================================================================
# Copyright 2004-2009 Kitware, Inc.
# Copyright 2008-2009 Alexander Neundorf <neundorf@kde.org>
# Copyright 2016      Luc Michel <luc.michel@git.antfield.fr>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

set(_RUBY_VERSIONS 2.3 2.2 2.1 2.0 1.9 1.8)

if(RubyInterp_FIND_VERSION)
    if(RubyInterp_FIND_VERSION_COUNT GREATER 1)
        set(_RUBY_FIND_MAJ_MIN "${RubyInterp_FIND_VERSION_MAJOR}.${RubyInterp_FIND_VERSION_MINOR}")
        list(APPEND _Ruby_NAMES
             ruby${_RUBY_FIND_MAJ_MIN}
             ruby${RubyInterp_FIND_VERSION_MAJOR})
        unset(_RUBY_FIND_OTHER_VERSIONS)
        if(NOT RubyInterp_FIND_VERSION_EXACT)
            foreach(_RUBY_V ${_RUBY_VERSIONS})
                if(NOT _RUBY_V VERSION_LESS _RUBY_FIND_MAJ_MIN)
                    list(APPEND _RUBY_FIND_OTHER_VERSIONS ${_RUBY_V})
                endif()
             endforeach()
        endif()
        unset(_RUBY_FIND_MAJ_MIN)
    else()
        list(APPEND _Ruby_NAMES ruby${RubyInterp_FIND_VERSION_MAJOR})
        set(_RUBY_FIND_OTHER_VERSIONS ${_RUBY${RubyInterp_FIND_VERSION_MAJOR}_VERSIONS})
    endif()
else()
    set(_RUBY_FIND_OTHER_VERSIONS ${_RUBY_VERSIONS})
endif()

find_program(RUBY_EXECUTABLE NAMES ${_Ruby_NAMES})

function(_RUBY_CONFIG_VAR RBVAR OUTVAR)
    execute_process(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['${RBVAR}']"
        RESULT_VARIABLE _RUBY_SUCCESS
        OUTPUT_VARIABLE _RUBY_OUTPUT
        ERROR_QUIET)
    if(_RUBY_SUCCESS OR NOT _RUBY_OUTPUT)
        execute_process(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['${RBVAR}']"
            RESULT_VARIABLE _RUBY_SUCCESS
            OUTPUT_VARIABLE _RUBY_OUTPUT
            ERROR_QUIET)
    endif()
    set(${OUTVAR} "${_RUBY_OUTPUT}" PARENT_SCOPE)
endfunction()

list(APPEND _Ruby_VERSIONS ";")
list(APPEND _Ruby_VERSIONS ${_RUBY_FIND_OTHER_VERSIONS})

# Search for newest ruby version if ruby executable isn't found
if(NOT RUBY_EXECUTABLE)
    foreach(_CURRENT_VERSION IN LISTS _Ruby_VERSIONS)
        set(_Ruby_NAMES ruby${_CURRENT_VERSION})
        list(APPEND _Ruby_NAMES ruby)
        find_program(RUBY_EXECUTABLE NAMES ${_Ruby_NAMES})
    endforeach()
endif()

if(RUBY_EXECUTABLE)
   _RUBY_CONFIG_VAR("MAJOR" RUBY_VERSION_MAJOR)
   _RUBY_CONFIG_VAR("MINOR" RUBY_VERSION_MINOR)
   _RUBY_CONFIG_VAR("TEENY" RUBY_VERSION_PATCH)

   set(RUBY_VERSION_MAJOR ${RUBY_VERSION_MAJOR} CACHE PATH "The Ruby major version" FORCE)
   set(RUBY_VERSION_MINOR ${RUBY_VERSION_MINOR} CACHE PATH "The Ruby minor version" FORCE)
   set(RUBY_VERSION_PATCH ${RUBY_VERSION_PATCH} CACHE PATH "The Ruby patch version" FORCE)

   set(RUBY_VERSION_STRING ${RUBY_VERSION_MAJOR}.${RUBY_VERSION_MINOR})

   if(RUBY_VERSION_PATCH)
       set(RUBY_VERSION_STRING ${RUBY_VERSION_STRING}.${RUBY_VERSION_PATCH})
   endif()

   mark_as_advanced(
       RUBY_VERSION_MAJOR
       RUBY_VERSION_MINOR
       RUBY_VERSION_PATCH
       RUBY_VERSION_STRING
       )
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RubyInterp REQUIRED_VARS RUBY_EXECUTABLE VERSION_VAR RUBY_VERSION_STRING)
