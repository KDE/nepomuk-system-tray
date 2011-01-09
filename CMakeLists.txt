project( nepomuk_systemtray )

cmake_minimum_required(VERSION 2.6)

set (QT_MIN_VERSION "4.6.0")
set (QT_USE_QTNETWORK true)
find_package(KDE4  REQUIRED)
find_package(Qt4  REQUIRED)
find_package(Nepomuk REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

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


