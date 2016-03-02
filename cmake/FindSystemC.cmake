#.rst:
# FindSystemC
# -----------
#
# Try to find SystemC

set(_SYSTEMC_PATHS PATHS
	${SYSTEMC_PREFIX}
	$ENV{SYSTEMC_PREFIX}
)

find_path(SYSTEMC_INCLUDE_DIR systemc ${_SYSTEMC_PATHS} PATH_SUFFIXES include)

if (NOT SYSTEMC_LIBRARIES)
	find_library(SYSTEMC_LIBRARY_RELEASE NAMES systemc ${_SYSTEMC_PATHS} PATH_SUFFIXES lib-linux lib-linux64 lib-mingw64)

	include(SelectLibraryConfigurations)
	SELECT_LIBRARY_CONFIGURATIONS(SYSTEMC)
endif ()

if (SYSTEMC_INCLUDE_DIR AND EXISTS "${SYSTEMC_INCLUDE_DIR}/sysc/kernel/sc_ver.h")
	file(STRINGS "${SYSTEMC_INCLUDE_DIR}/sysc/kernel/sc_ver.h" _sc_ver REGEX "^#define SC_VERSION_.*")
	string(REGEX REPLACE ".*#define SC_VERSION_MAJOR[\t ]+([0-9]+).*" "\\1" SYSTEMC_MAJOR "${_sc_ver}")
	string(REGEX REPLACE ".*#define SC_VERSION_MINOR[\t ]+([0-9]+).*" "\\1" SYSTEMC_MINOR "${_sc_ver}")
	string(REGEX REPLACE ".*#define SC_VERSION_PATCH[\t ]+([0-9]+).*" "\\1" SYSTEMC_PATCH "${_sc_ver}")

	set(SYSTEMC_VERSION "${SYSTEMC_MAJOR}.${SYSTEMC_MINOR}.${SYSTEMC_PATCH}")
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SYSTEMC
	                          REQUIRED_VARS SYSTEMC_LIBRARIES SYSTEMC_INCLUDE_DIR
				  VERSION_VAR SYSTEMC_VERSION)
