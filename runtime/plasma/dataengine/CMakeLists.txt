project(plasma-nepomukengine)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

set( nepomukengine_SRCS
    nepomuk_engine.cpp
    )

# Name must match value of X-KDE-Library in .desktop file 
kde4_add_plugin( plasma_engine_nepomuk
    ${nepomukengine_SRCS}
    )

target_link_libraries( plasma_engine_nepomuk
    ${KDE4_KDECORE_LIBS}
    ${KDE4_PLASMA_LIBS}
    nepomuk_systray
    )

install(TARGETS plasma_engine_nepomuk
            DESTINATION ${PLUGIN_INSTALL_DIR})
         
install(FILES plasma-engine-nepomuk.desktop
            DESTINATION ${SERVICES_INSTALL_DIR})



