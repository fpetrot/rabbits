rabbits_add_sources(
	ui.cc
    tester.cc
    view.cc
)

if(RABBITS_CONFIG_QT_FRAMEBUFFER)
    rabbits_add_sources(surface.cc)
endif()

add_subdirectory(view)
