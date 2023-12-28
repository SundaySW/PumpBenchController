cmake_minimum_required(VERSION 3.5)

#set(CMAKE_TOOLCHAIN_FILE cmake/toolchain.cmake)

project(PumpBenchController VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("SysName: ${CMAKE_SYSTEM_NAME}")

set(qt_ver      Qt5)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(${qt_ver}Widgets       REQUIRED)
find_package(${qt_ver}Gui           REQUIRED)
find_package(${qt_ver}Core          REQUIRED)
find_package(${qt_ver}Network       REQUIRED)
find_package(${qt_ver}SerialPort    REQUIRED)
find_package(${qt_ver}Sql           REQUIRED)
find_package(${qt_ver}Svg           REQUIRED)
find_package(${qt_ver}Xml           REQUIRED)
find_package(${qt_ver}PrintSupport  REQUIRED)

find_package(${qt_ver}OpenGL        REQUIRED)
find_package(OpenGL                 REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/ParamService/*.*
        ${CMAKE_CURRENT_SOURCE_DIR}/BenchItem/*.*
        ${CMAKE_CURRENT_SOURCE_DIR}/BenchItem/*.*
        ${CMAKE_CURRENT_SOURCE_DIR}/PID/*.*
        ${CMAKE_CURRENT_SOURCE_DIR}/Dialogs/*.*
        ${CMAKE_CURRENT_SOURCE_DIR}/Scenarios/*.*
        ${CMAKE_CURRENT_SOURCE_DIR}/Experiments/*.*
        ${CMAKE_CURRENT_SOURCE_DIR}/KeyPad/*.*
        )

set(PROJECT_SOURCES
        ${SOURCES}
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        Resources/files.qrc
        BenchController.cpp
        BenchController.h
        colors.h
        config.h.in
        )

configure_file(
        config.h.in config.h)

add_subdirectory(qcustomplot)

add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        )

target_sources(${PROJECT_NAME}
        PRIVATE
        ${SOURCE_FILES}
        )

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ParamService
        ${CMAKE_CURRENT_SOURCE_DIR}/BenchItem
        ${CMAKE_CURRENT_SOURCE_DIR}/PID
        ${CMAKE_CURRENT_SOURCE_DIR}/Dialogs
        ${CMAKE_CURRENT_SOURCE_DIR}/Scenarios
        ${CMAKE_CURRENT_SOURCE_DIR}/Experiments
        ${CMAKE_CURRENT_SOURCE_DIR}/KeyPad
        ${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
    set (WIN32_RESOURCES Resources/resource.rc)
endif()

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("Debug build!")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/saved.json ${CMAKE_CURRENT_BINARY_DIR}/saved.json COPYONLY)
    add_custom_command(
            TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Resources/ ${CMAKE_CURRENT_BINARY_DIR}/Resources/
    )
    if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot/bin ${CMAKE_CURRENT_BINARY_DIR}/
        )
    endif()
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES Release)
    message("Release build!")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/saved.json ${CMAKE_CURRENT_BINARY_DIR}/saved.json COPYONLY)
    add_custom_command(
            TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Resources/ ${CMAKE_CURRENT_BINARY_DIR}/Resources/
    )
    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/Makefile
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/.cmake
    )
    if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE true)
        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND windeployqt "$<TARGET_FILE:${PROJECT_NAME}>"
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot/bin ${CMAKE_CURRENT_BINARY_DIR}/
                VERBATIM
        )
    endif()
    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/qcustomplot/ ${CMAKE_CURRENT_BINARY_DIR}/
        )
    endif()
ENDIF()

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${qt_ver}::Core
        ${qt_ver}::Gui
        ${qt_ver}::Widgets
        ${qt_ver}::Network
        ${qt_ver}::SerialPort
        ${qt_ver}::Sql
        ${qt_ver}::Svg
        ${qt_ver}::Xml
        ${qt_ver}::Widgets qcustomplot
        ${qt_ver}::OpenGL
        ${qt_ver}::PrintSupport
        ${OPENGL_LIBRARIES}
        )

target_compile_definitions(${PROJECT_NAME} PRIVATE
    QCUSTOMPLOT_USE_OPENGL
    QCUSTOMPLOT_USE_LIBRARY
)

install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PumpBenchController)
endif()

#target_compile_options(${PROJECT_NAME}
#        PRIVATE
#        -Wall
#        -Wextra
#        -Wpedantic
##        -Werror
#        -Wduplicated-cond
#        -Wduplicated-branches
#        -Wlogical-op
#        -Wnull-dereference
#        -Wshadow
#        -Wconversion
#        -Wsign-conversion
#        )