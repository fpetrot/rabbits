add_subdirectory(dummy)

if(RABBITS_CONFIG_QT)
    add_subdirectory(qt)
endif()

rabbits_add_sources(
	ui.cc
    chooser.cc
)
