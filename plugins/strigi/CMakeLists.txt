include_directories(
    ${nepomuk_systemtray_SOURCE_DIR}/lib
    ${nepomuk_systemtray_BINARY_DIR}/lib
    )

set( NST_ST_SOURCES
    strigiplugin.cpp
    )

# Adding interfaces
qt4_add_dbus_interface(NST_ST_SOURCES ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.nepomuk.Strigi.xml strigiserviceinterface)

kde4_add_plugin( nepomuksystray_strigi ${NST_ST_SOURCES} )

target_link_libraries( nepomuksystray_strigi
    ${QT_LIBRARIES}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    nepomuk_systray
    )

install(
  TARGETS nepomuksystray_strigi
  DESTINATION ${PLUGIN_INSTALL_DIR}
  )

install(
  FILES nepomuksystray_strigi.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/nepomukcontroller
  )

install(FILES systraystrigipluginui.rc 
           DESTINATION  ${DATA_INSTALL_DIR}/nepomukcontroller) 
