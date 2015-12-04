#
#  This file is part of Rabbits
#  Copyright (C) 2015  Clement Deschamps and Luc Michel
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


# GCC flags
set(GNU_C_FLAGS_Release "")
set(GNU_CXX_FLAGS_Release "")

set(GNU_C_FLAGS_Debug "-Wall -Wextra -Wno-unused-parameter")
set(GNU_CXX_FLAGS_Debug "-Wall -Wextra -Wno-unused-parameter")

# Clang flags
set(Clang_C_FLAGS_Release "${GNU_C_FLAGS_Release}")
set(Clang_CXX_FLAGS_Release "${GNU_CXX_FLAGS_Release}")

set(Clang_C_FLAGS_Debug "${GNU_C_FLAGS_Debug}")
set(Clang_CXX_FLAGS_Debug "${GNU_CXX_FLAGS_Debug}")

# MSVC flags (TODO)
set(MSVC_C_FLAGS_Release "")
set(MSVC_CXX_FLAGS_Release "")

set(MSVC_C_FLAGS_Debug "")
set(MSVC_CXX_FLAGS_Debug "")



set(__extra_c_flags ${${CMAKE_C_COMPILER_ID}_C_FLAGS_${CMAKE_BUILD_TYPE}})
set(__extra_cxx_flags ${${CMAKE_CXX_COMPILER_ID}_CXX_FLAGS_${CMAKE_BUILD_TYPE}})


if (__extra_c_flags)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${__extra_c_flags}")
endif()

if (__extra_cxx_flags)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${__extra_cxx_flags}")
endif()
