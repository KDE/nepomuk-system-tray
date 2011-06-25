project( nepomuk_systemtray )

cmake_minimum_required(VERSION 2.6)

set (QT_MIN_VERSION "4.6.0")
set (QT_USE_QTNETWORK true)
find_package(KDE4  REQUIRED)
find_package(Qt4  REQUIRED)
find_package(Nepomuk REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

# Nepomuk-system-tray has a build dependency on kdebase-runtime.
# We must look for org.kde.nepomuk.(ServiceManager.xml|ServiceControl.xml)
find_path(kde-runtime_DBUS_DIR
     NAMES org.kde.nepomuk.ServiceManager.xml org.kde.nepomuk.ServiceControl.xml
     HINTS ${KDE4_DBUS_INTERFACES_DIR} ${DBUS_INTERFACES_INSTALL_DIR})
message(STATUS "DEBUG ${kde-runtime_DBUS_DIR}")

if(NOT kde-runtime_DBUS_DIR)
  message(FATAL_ERROR "Could not find the kdebase-runtime DBUS interfaces.")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


include_directories(
    ${CMAKE_CURRENT_BINARY_DIR} 
    ${CMAKE_CURRENT_BINARY_DIR}/lib 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib 
    ${NEPOMUK_INCLUDE_DIR}
    ${KDE4_INCLUDES}
    )

add_subdirectory(lib)
add_subdirectory(plugins)
add_subdirectory(runtime)


