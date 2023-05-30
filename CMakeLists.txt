cmake_minimum_required(VERSION 3.5)

project(PumpBenchController VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(qt_ver      Qt5)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(${qt_ver}Widgets     REQUIRED)
find_package(${qt_ver}Gui         REQUIRED)
find_package(${qt_ver}Core        REQUIRED)
find_package(${qt_ver}Network     REQUIRED)
find_package(${qt_ver}SerialPort  REQUIRED)
find_package(${qt_ver}Sql         REQUIRED)
find_package(${qt_ver}Svg         REQUIRED)
find_package(${qt_ver}Xml         REQUIRED)
# For OPENGl
find_package(OpenGL REQUIRED)
find_package(${qt_ver} REQUIRED COMPONENTS Widgets OpenGL)
# end For OPENGl
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS PrintSupport)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/ParamService/*.*
        ${CMAKE_CURRENT_SOURCE_DIR}/BenchItem/*.*
        ${CMAKE_CURRENT_SOURCE_DIR}/BenchItem/*.*
        ${CMAKE_CURRENT_SOURCE_DIR}/PID/*.*
        ${CMAKE_CURRENT_SOURCE_DIR}/Dialogs/*.*
        #        ${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot/*.*
        )

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        Resources/files.qrc
        BenchController.cpp
        BenchController.h
        ${SOURCES}
        colors.h)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
    set (WIN32_RESOURCES Resources/resource.rc)
endif()

IF(CMAKE_BUILD_TYPE MATCHES Release)
    message("Release build!")
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE true)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/saved_params.json ${CMAKE_CURRENT_BINARY_DIR}/saved_params.json COPYONLY)
    add_custom_command(
            TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Resource/ ${CMAKE_CURRENT_BINARY_DIR}/Resource/
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Resources/ ${CMAKE_CURRENT_BINARY_DIR}/Resources/
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/deploy_libs ${CMAKE_CURRENT_BINARY_DIR}/
    )
    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND windeployqt "$<TARGET_FILE:${PROJECT_NAME}>"
            VERBATIM
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