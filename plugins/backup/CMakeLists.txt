## Include directories. If your plugin use extra libraries,
## then add them here.
## ATTENTION: If you use extra Qt libraries, then don't insert
## find_package(Qt4 <blablabla>) here. INSTEAD modify top-lvel 
## CMakeLists ( aka <repository>/CMakeLists.txt )
include_directories(
    ${nepomuk_systemtray_SOURCE_DIR}/lib
    ${nepomuk_systemtray_BINARY_DIR}/lib
    )

## This is variable with sources of plugin
## If your plugin use more source files, add them here
set( NST_nepomukbackupsync_SOURCES
    nepomukbackupsyncplugin.cpp
    )

## Adding interfaces. Most likely, you will need to create a header/source pair
## with DBus interface to your service. Insert necessary name manualy here
## or comment this line out
#qt4_add_dbus_interface(NST_nepomukbackupsync_SOURCES ${kde-runtime_DBUS_DIR}/org.kde.nepomuk.XXXXX.xml nepomukbackupsyncserviceinterface)
#

kde4_add_plugin( nepomuksystray_nepomukbackupsync ${NST_nepomukbackupsync_SOURCES} )

# Link with necessary libraries.
target_link_libraries( nepomuksystray_nepomukbackupsync
    ${QT_LIBRARIES}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    nepomuk_systray
    )

install(
  TARGETS nepomuksystray_nepomukbackupsync
  DESTINATION ${PLUGIN_INSTALL_DIR}
  )

install(
  FILES nepomuksystray_nepomukbackupsync.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/nepomuksystray_qt
  )

install(FILES systraynepomukbackupsyncpluginui.rc 
           DESTINATION  ${DATA_INSTALL_DIR}/nepomuksystray_qt) 

