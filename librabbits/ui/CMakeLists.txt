add_subdirectory(dummy)

if(RABBITS_CONFIG_SDL)
	add_subdirectory(sdl)
endif()

rabbits_add_sources(
	ui.cc
)
