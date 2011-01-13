
add_subdirectory(kcm)

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

kde4_add_executable( nepomukcontroller ${NST_SOURCES} )

target_link_libraries(nepomukcontroller
    nepomuk_systray
    ${KDE4_KDEUI_LIBS}
    )


install(
  FILES nepomukcontroller.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR})

install( TARGETS nepomukcontroller ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES nepomukcontrollerui.rc 
           DESTINATION  ${DATA_INSTALL_DIR}/nepomukcontroller) 
