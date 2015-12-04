rabbits_add_sources(
	dynloader.cc
	dynlib.cc
)

if(RABBITS_CONFIG_POSIX)
	rabbits_add_sources(dynlib_posix.cc)
	rabbits_add_libs(dl)
elseif(RABBITS_CONFIG_WIN32)
	rabbits_add_sources(dynlib_win32.cc)
endif()
