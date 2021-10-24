cmake_minimum_required(VERSION 3.11)

project(scwx-qt LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost)
find_package(Freetype)
find_package(geographiclib)
find_package(glm)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6
             COMPONENTS Gui
                        LinguistTools
                        Network
                        OpenGL
                        OpenGLWidgets
                        Widgets REQUIRED)

find_package(Qt${QT_VERSION_MAJOR}
             COMPONENTS Gui
                        LinguistTools
                        Network
                        OpenGL
                        OpenGLWidgets
                        Widgets
             REQUIRED)

include(qt6-linguist.cmake)

set(SRC_EXE_MAIN source/scwx/qt/main/main.cpp)

set(HDR_MAIN source/scwx/qt/main/main_window.hpp)
set(SRC_MAIN source/scwx/qt/main/main_window.cpp)
set(UI_MAIN  source/scwx/qt/main/main_window.ui)
set(HDR_GL source/scwx/qt/gl/gl.hpp
           source/scwx/qt/gl/shader_program.hpp
           source/scwx/qt/gl/text_shader.hpp)
set(SRC_GL source/scwx/qt/gl/shader_program.cpp
           source/scwx/qt/gl/text_shader.cpp)
set(HDR_MANAGER source/scwx/qt/manager/radar_manager.hpp
                source/scwx/qt/manager/resource_manager.hpp
                source/scwx/qt/manager/settings_manager.hpp)
set(SRC_MANAGER source/scwx/qt/manager/radar_manager.cpp
                source/scwx/qt/manager/resource_manager.cpp
                source/scwx/qt/manager/settings_manager.cpp)
set(HDR_MAP source/scwx/qt/map/map_widget.hpp
            source/scwx/qt/map/overlay_layer.hpp
            source/scwx/qt/map/radar_layer.hpp
            source/scwx/qt/map/radar_range_layer.hpp
            source/scwx/qt/map/triangle_layer.hpp)
set(SRC_MAP source/scwx/qt/map/map_widget.cpp
            source/scwx/qt/map/overlay_layer.cpp
            source/scwx/qt/map/radar_layer.cpp
            source/scwx/qt/map/radar_range_layer.cpp
            source/scwx/qt/map/triangle_layer.cpp)
set(HDR_SETTINGS source/scwx/qt/settings/general_settings.hpp)
set(SRC_SETTINGS source/scwx/qt/settings/general_settings.cpp)
set(HDR_UTIL source/scwx/qt/util/font.hpp
             source/scwx/qt/util/font_buffer.hpp
             source/scwx/qt/util/json.hpp)
set(SRC_UTIL source/scwx/qt/util/font.cpp
             source/scwx/qt/util/font_buffer.cpp
             source/scwx/qt/util/json.cpp)
set(HDR_VIEW source/scwx/qt/view/radar_view.hpp)
set(SRC_VIEW source/scwx/qt/view/radar_view.cpp)

set(RESOURCE_FILES scwx-qt.qrc)

set(SHADER_FILES gl/overlay.frag
                 gl/overlay.vert
                 gl/radar.frag
                 gl/radar.vert
                 gl/text.frag
                 gl/text.vert)

set(TS_FILES ts/scwx_en_US.ts)

set(PROJECT_SOURCES ${HDR_MAIN}
                    ${SRC_MAIN}
                    ${HDR_GL}
                    ${SRC_GL}
                    ${HDR_MANAGER}
                    ${SRC_MANAGER}
                    ${UI_MAIN}
                    ${HDR_MAP}
                    ${SRC_MAP}
                    ${HDR_SETTINGS}
                    ${SRC_SETTINGS}
                    ${HDR_UTIL}
                    ${SRC_UTIL}
                    ${HDR_VIEW}
                    ${SRC_VIEW}
                    ${SHADER_FILES}
                    ${RESOURCE_FILES}
                    ${TS_FILES})
set(EXECUTABLE_SOURCES ${SRC_EXE_MAIN})

source_group("Header Files\\main"     FILES ${HDR_MAIN})
source_group("Source Files\\main"     FILES ${SRC_MAIN})
source_group("Header Files\\gl"       FILES ${HDR_GL})
source_group("Source Files\\gl"       FILES ${SRC_GL})
source_group("Header Files\\manager"  FILES ${HDR_MANAGER})
source_group("Source Files\\manager"  FILES ${SRC_MANAGER})
source_group("UI Files\\main"         FILES ${UI_MAIN})
source_group("Header Files\\map"      FILES ${HDR_MAP})
source_group("Source Files\\map"      FILES ${SRC_MAP})
source_group("Header Files\\settings" FILES ${HDR_SETTINGS})
source_group("Source Files\\settings" FILES ${SRC_SETTINGS})
source_group("Header Files\\util"     FILES ${HDR_UTIL})
source_group("Source Files\\util"     FILES ${SRC_UTIL})
source_group("Header Files\\view"     FILES ${HDR_VIEW})
source_group("Source Files\\view"     FILES ${SRC_VIEW})
source_group("OpenGL Shaders"         FILES ${SHADER_FILES})
source_group("Resources"              FILES ${RESOURCE_FILES})
source_group("I18N Files"             FILES ${TS_FILES})

add_library(scwx-qt OBJECT ${PROJECT_SOURCES})
set_property(TARGET scwx-qt PROPERTY AUTOMOC ON)

qt_add_executable(supercell-wx ${EXECUTABLE_SOURCES})

qt6_create_translation_scwx(QM_FILES ${scwx-qt_SOURCE_DIR} ${TS_FILES})

if (WIN32)
    target_compile_definitions(scwx-qt      PUBLIC WIN32_LEAN_AND_MEAN)
    target_compile_definitions(supercell-wx PUBLIC WIN32_LEAN_AND_MEAN)
endif()

target_include_directories(scwx-qt PUBLIC ${scwx-qt_SOURCE_DIR}/source
                                          ${FTGL_INCLUDE_DIR}
                                          ${MBGL_INCLUDE_DIR})

target_include_directories(supercell-wx PUBLIC ${scwx-qt_SOURCE_DIR}/source)

target_link_libraries(scwx-qt PUBLIC Qt${QT_VERSION_MAJOR}::Widgets
                                     Qt${QT_VERSION_MAJOR}::OpenGLWidgets
                                     Boost::json
                                     Boost::timer
                                     qmapboxgl
                                     opengl32
                                     freetype-gl
                                     GeographicLib::GeographicLib
                                     glm::glm
                                     wxdata)

target_link_libraries(supercell-wx PRIVATE scwx-qt
                                           wxdata)
