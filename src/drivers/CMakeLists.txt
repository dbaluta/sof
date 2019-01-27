if(CONFIG_BAYTRAIL)
add_local_sources(sof dw-dma.c)

add_subdirectory(intel)
else(CONFIG_IMX8)
add_subdirectory(imx)
endif()
