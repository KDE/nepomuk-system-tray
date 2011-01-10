
add_subdirectory(kcm)

## Add program
set ( NST_SOURCES 
    main.cpp
    systray.cpp
    )

kde4_add_executable( nepomuk_system_tray ${NST_SOURCES} )

target_link_libraries(nepomuk_system_tray
    nepomuk_systray
    ${KDE4_KDEUI_LIBS}
    )


install( TARGETS nepomuk_system_tray ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES nepomukcontrollerui.rc 
           DESTINATION  ${DATA_INSTALL_DIR}/nepomukcontroller) 
