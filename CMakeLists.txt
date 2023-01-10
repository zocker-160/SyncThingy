cmake_minimum_required(VERSION 3.16)
project(SyncThingy)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")

# BS workaround for stupid Glib using "signals" for which QT uses as well
add_definitions(-DQT_NO_KEYWORDS)

find_package(PkgConfig REQUIRED)
#pkg_check_modules(PORTAL REQUIRED IMPORTED_TARGET libportal)
pkg_check_modules(PORTAL-QT REQUIRED IMPORTED_TARGET libportal-qt5)

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp src/SettingsDialog.cpp src/SettingsDialog.h src/Constants.h)
add_subdirectory(src/singleapplication)

target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        PkgConfig::PORTAL-QT
        SingleApplication::SingleApplication)

if (STATIC)
    #Static start
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_SEARCH_START_STATIC 1)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_SEARCH_END_STATIC 1)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(Qt5_USE_STATIC_LIBS ON)
    set(Qt5_USE_STATIC_RUNTIME ON)
    #Static Libs

    #Set Linker flags
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
