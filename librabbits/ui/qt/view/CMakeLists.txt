rabbits_add_sources(
	webkit.cc
)

if(RABBITS_CONFIG_QT_FRAMEBUFFER)
    rabbits_add_sources(framebuffer.cc)
endif()
