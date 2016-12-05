add_subdirectory(dummy)

if(RABBITS_CONFIG_SDL)
	add_subdirectory(sdl)
endif()

if(RABBITS_CONFIG_QT)
	add_subdirectory(qt)
endif()

rabbits_add_sources(
	ui.cc
)
