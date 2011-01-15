add_subdirectory(strigi)

install(
  FILES nepomuksystray_plugin.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
  )

macro(BUILDINSTALL_STANDARD_PLUGIN _servicedesktopfile _dbusname _service_description _pluginsystemname )
  set(_dbusname ${_dbusname} )
  set(_servicedesktopfile ${_servicedesktopfile} )
  set(_service_description ${_service_description} )
  set(_pluginsystemname ${_pluginsystemname} )
  configure_file("basic_template/templateplugin.h.in" "${CMAKE_CURRENT_BINARY_DIR}/${_dbusname}plugin.h")
  configure_file("basic_template/templateplugin.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/${_dbusname}plugin.cpp")
  configure_file("basic_template/nepomuksystray_template.desktop.in" "${CMAKE_CURRENT_BINARY_DIR}/nepomuksystray_${_dbusname}.desktop")
  configure_file("basic_template/systraytemplatepluginui.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/systray${_dbusname}pluginui.rc")
  kde4_add_plugin( nepomuksystray_${_dbusname} ${CMAKE_CURRENT_BINARY_DIR}/${_dbusname}plugin.cpp )
  target_link_libraries( nepomuksystray_${_dbusname}
    ${QT_LIBRARIES}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    nepomuk_systray
    )
    install(
      TARGETS nepomuksystray_${_dbusname}
      DESTINATION ${PLUGIN_INSTALL_DIR}
      )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/nepomuksystray_${_dbusname}.desktop
      DESTINATION ${SERVICES_INSTALL_DIR}/nepomukcontroller
      )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/systray${_dbusname}pluginui.rc 
               DESTINATION  ${DATA_INSTALL_DIR}/nepomukcontroller) 
endmacro(BUILDINSTALL_STANDARD_PLUGIN)

BUILDINSTALL_STANDARD_PLUGIN( "nepomukqueryservice.desktop" nepomukqueryservice "" "Nepomuk query service ")
BUILDINSTALL_STANDARD_PLUGIN( "nepomukstorage.desktop" nepomukstorage "" "Nepomuk storage ")
BUILDINSTALL_STANDARD_PLUGIN( "nepomukfilewatch.desktop" nepomukremovablestorageservice "" "Nepomuk removable storage service")
