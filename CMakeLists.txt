project(plasma-emergelog)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_BINARY_DIR}
	${KDE4_INCLUDES}
)

set(emergelog_SRCS plasma-emergelog.cpp plasma-emergelog-painter.cpp)

kde4_add_plugin(plasma_applet_emergelog ${emergelog_SRCS})

target_link_libraries(plasma_applet_emergelog ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

kde4_add_app_icon(emergelog_SRCS, "plasma-emergelog.png")
install(TARGETS plasma_applet_emergelog DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-emergelog.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-emergelog.png DESTINATION ${DATA_INSTALL_DIR}/plasma-emergelog/ )
