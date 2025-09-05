cmake_minimum_required(VERSION 3.16)

project(SyncThingy LANGUAGES CXX)
set(FLATPAK_APPID "com.github.zocker_160.SyncThingy")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")

# BS workaround for stupid Glib using "signals" that QT uses as well
add_definitions(-DQT_NO_KEYWORDS)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAL-QT REQUIRED IMPORTED_TARGET libportal-qt6)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        DBus
        REQUIRED
)

add_subdirectory(src/singleapplication)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/SettingsDialog.cpp
        src/ServiceDialog.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::DBus
        PkgConfig::PORTAL-QT
        SingleApplication
)


include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES linux_packaging/SyncThingy.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES icons/color.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps
)
install(FILES icons/white.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps
        RENAME ${FLATPAK_APPID}.white.png
)
install(FILES icons/black.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps
        RENAME ${FLATPAK_APPID}.black.png
)
install(FILES linux_packaging/metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
        RENAME ${PROJECT_NAME}.xml
)