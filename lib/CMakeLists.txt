# Add library
set( LIB_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

set( NSTL_SOURCES
    systrayplugin.cpp
    systraypluginmanager.cpp
    )

set ( NSTL_HEADERS
    systrayplugin.h
    systraypluginmanager.h
    nepomuksystray_export.h
    )

qt4_add_dbus_interface(NSTL_SOURCES ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.nepomuk.ServiceManager.xml servicemanagerinterface)
qt4_add_dbus_interface(NSTL_SOURCES ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.nepomuk.ServiceControl.xml servicecontrolinterface)

kde4_add_library( nepomuk_systray SHARED ${NSTL_SOURCES})
target_link_libraries(nepomuk_systray
  ${QT_LIBRARIES}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  )

install(TARGETS nepomuk_systray ${INSTALL_TARGETS_DEFAULT_ARGS} ) 
install(
    FILES ${NSTL_HEADERS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include  COMPONENT Devel
    )
    
