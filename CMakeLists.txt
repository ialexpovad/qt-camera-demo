cmake_minimum_required(VERSION 3.21)
project(QtCameraDemo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)
find_package(Qt6 COMPONENTS Permissions QUIET)

add_executable(QtCameraDemo
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/CameraWidget.h
    src/CameraWidget.cpp
    src/PlatformPermissions.h
    src/PlatformPermissions.cpp
    src/StaticPlugins.cpp
)

target_link_libraries(QtCameraDemo PRIVATE
    Qt::Widgets
    Qt::Multimedia
    Qt::MultimediaWidgets
)

if (TARGET Qt6::Permissions)
    target_link_libraries(QtCameraDemo PRIVATE Qt6::Permissions)
    qt_import_plugins(QtCameraDemo INCLUDE_BY_TYPE permissions)
else()
    message(STATUS "Qt6::Permissions NOT found (Homebrew case). Linking darwin permission plugin statically.")
    if(APPLE)
        set(_qt_perm_hints
            "$ENV{QT_PLUGIN_PATH}/permissions"
            "/opt/homebrew/opt/qt/plugins/permissions"
            "/usr/local/opt/qt/plugins/permissions"
            "/opt/homebrew/Cellar/qt/*/share/qt/plugins/permissions"
            "/usr/local/Cellar/qt/*/share/qt/plugins/permissions"
        )
        find_library(QDARWIN_CAMERA_PERMISSION_PLUGIN
            NAMES qdarwincamerapermission
            HINTS ${_qt_perm_hints}
            NO_DEFAULT_PATH
        )
        if (QDARWIN_CAMERA_PERMISSION_PLUGIN)
            target_compile_definitions(QtCameraDemo PRIVATE QT_STATICPLUGIN=1)
            target_link_libraries(QtCameraDemo PRIVATE "${QDARWIN_CAMERA_PERMISSION_PLUGIN}")
            if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
                target_link_options(QtCameraDemo PRIVATE "-Wl,-force_load,${QDARWIN_CAMERA_PERMISSION_PLUGIN}")
            endif()
        else()
            message(WARNING "libqdarwincamerapermission.a not found; camera permission may fail.")
        endif()
    endif()
endif()

if(WIN32)
  target_compile_definitions(QtCameraDemo PRIVATE NOMINMAX)
  set_target_properties(QtCameraDemo PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
  set(BUNDLE_ID "com.ialexpobad.qtcamerademo")
  set(PROJECT_VERSION "${PROJECT_VERSION}")

  target_sources(QtCameraDemo PRIVATE src/PlatformPermissions_mac.mm)
  set_source_files_properties(src/PlatformPermissions_mac.mm PROPERTIES COMPILE_FLAGS "-fobjc-arc")
  target_link_libraries(QtCameraDemo PRIVATE "-framework AVFoundation" "-framework Foundation")

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos/Info.plist.in
                 ${CMAKE_CURRENT_BINARY_DIR}/Info.plist @ONLY)

  set_target_properties(QtCameraDemo PROPERTIES
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_GUI_IDENTIFIER "${BUNDLE_ID}"
      MACOSX_BUNDLE_BUNDLE_NAME "QtCameraDemo"
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
  )
endif()
