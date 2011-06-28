
#add_subdirectory(kcm)

## Add program
set ( NST_SOURCES 
    main.cpp
    systray.cpp
    mainwidget.cpp
    servicewidget.cpp
    )

kde4_add_ui_files( NST_SOURCES 
    mainWidget.ui
    serviceWidget.ui
    )

kde4_add_executable( nepomuksystray_qt ${NST_SOURCES} )

target_link_libraries(nepomuksystray_qt
    nepomuk_systray
    ${KDE4_KDEUI_LIBS}
    )


install(
  FILES nepomuksystray_qt.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR})

install( TARGETS nepomuksystray_qt ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES nepomuksystray_qtui.rc 
           DESTINATION  ${DATA_INSTALL_DIR}/nepomuksystray_qt) 
install (
    FILES nepomuksystray_qtrc DESTINATION ${CONFIG_INSTALL_DIR}
    )

