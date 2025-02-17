# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------- CMAKE PROJECT
# ---------------------------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0054 NEW)

project(getopt-for-windows LANGUAGES CXX C)

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------- CMAKE OPTIONS
# ---------------------------------------------------------------------------------------------------------------------

option(OPT_COMPILE_STATIC_LIB   "Compile the static library" ON )
option(OPT_COMPILE_SHARED_LIB   "Compile the shared library" ON)
option(OPT_STATUS_MESSAGE_QUIET "Quiet the status messages"  OFF)

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------- CMAKE CONFIGURATION
# ---------------------------------------------------------------------------------------------------------------------
message(STATUS "getopt-for-windows - cmakelist")


# set CMAKE_INSTALL_BINDIR, CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_INCLUDEDIR and CMAKE_INSTALL_DATAROOTDIR variables
include(GNUInstallDirs)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Specify lib path
set(LIBRARY_OUTPUT_PATH lib/)

# Generate the source files lists
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------- OBJECTS/LIBS/EXE
# ---------------------------------------------------------------------------------------------------------------------

# Object
add_library(getopt-for-windows-obj OBJECT ${SRC_FILES})
set_target_properties(getopt-for-windows-obj PROPERTIES
                              POSITION_INDEPENDENT_CODE ON) # set -fpic

# Shared library
if(OPT_COMPILE_SHARED_LIB)
    add_library(getopt-for-windows-shared-lib SHARED $<TARGET_OBJECTS:getopt-for-windows-obj>)
    set_target_properties(getopt-for-windows-shared-lib PROPERTIES
                                         OUTPUT_NAME getopt-for-windows
                                         POSITION_INDEPENDENT_CODE ON) # set -fpic
    if(NOT getopt-for-windows_STATUS_MESSAGE_QUIET)
        message(STATUS "getopt-for-windows - Compile: shared library")
    endif()
endif(OPT_COMPILE_SHARED_LIB)

# Static library
if(OPT_COMPILE_STATIC_LIB)
    add_library(getopt-for-windows-static-lib STATIC $<TARGET_OBJECTS:getopt-for-windows-obj>)
    set_target_properties(getopt-for-windows-static-lib PROPERTIES
                                         OUTPUT_NAME getopt-for-windows
                                         POSITION_INDEPENDENT_CODE ON) # set -fpic
    if(NOT OPT_STATUS_MESSAGE_QUIET)
        message(STATUS "getopt-for-windows - Compile: static library")
    endif()
endif(OPT_COMPILE_STATIC_LIB)

# ---------------------------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------- HEADER ONLY LIBRARIES
# ---------------------------------------------------------------------------------------------------------------------

# getopt-for-windows
target_include_directories(getopt-for-windows-obj PRIVATE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/getopt-for-windows/>)
if(OPT_COMPILE_SHARED_LIB)
    target_include_directories(getopt-for-windows-shared-lib PRIVATE
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/getopt-for-windows/>)
endif(OPT_COMPILE_SHARED_LIB)
if(OPT_COMPILE_STATIC_LIB)
    target_include_directories(getopt-for-windows-static-lib PRIVATE
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/getopt-for-windows/>)
endif(OPT_COMPILE_STATIC_LIB)

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------ COMPILER DEFINITIONS
# ---------------------------------------------------------------------------------------------------------------------

if(WIN32) # for Windows operating system in general
    set(WINDOWS_VISTA 0x0600)
    target_compile_definitions(getopt-for-windows-obj PUBLIC $<BUILD_INTERFACE:_WIN32_WINNT=${WINDOWS_VISTA}>
                                              $<INSTALL_INTERFACE:_WIN32_WINNT=${WINDOWS_VISTA}>)
    target_compile_definitions(getopt-for-windows-obj PUBLIC $<BUILD_INTERFACE:NOMINMAX>
                                              $<INSTALL_INTERFACE:NOMINMAX>)
endif()

# ---------------------------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------------------- EXPORT
# ---------------------------------------------------------------------------------------------------------------------

if (OPT_COMPILE_SHARED_LIB AND NOT OPT_COMPILE_STATIC_LIB)
    export(TARGETS
           getopt-for-windows-shared-lib
           NAMESPACE getopt-for-windows::
           FILE "lib/cmake/getopt-for-windows/getopt-for-windows-config.cmake")
endif()

if (OPT_COMPILE_STATIC_LIB AND NOT OPT_COMPILE_SHARED_LIB)
    export(TARGETS
           getopt-for-windows-static-lib
           NAMESPACE getopt-for-windows::
           FILE "lib/cmake/getopt-for-windows/getopt-for-windows-config.cmake")
endif()

if(OPT_COMPILE_SHARED_LIB AND OPT_COMPILE_STATIC_LIB)
    export(TARGETS
           getopt-for-windows-shared-lib
           getopt-for-windows-static-lib
           NAMESPACE getopt-for-windows::
           FILE "lib/cmake/getopt-for-windows/getopt-for-windows-config.cmake")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------- INSTALL
# ---------------------------------------------------------------------------------------------------------------------

if(OPT_COMPILE_SHARED_LIB)
    if(WIN32)
        install(TARGETS getopt-for-windows-shared-lib
                EXPORT getopt-for-windows-config
                RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
                COMPONENT library)
    else()
        install(TARGETS getopt-for-windows-shared-lib
                EXPORT getopt-for-windows-config
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
                COMPONENT library)
    endif()
endif()
if(OPT_COMPILE_STATIC_LIB)
    install(TARGETS getopt-for-windows-static-lib
            EXPORT getopt-for-windows-config
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
            COMPONENT library)
endif()

if (OPT_COMPILE_SHARED_LIB OR OPT_COMPILE_STATIC_LIB)
    install(EXPORT
            getopt-for-windows-config
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/getopt-for-windows/"
            NAMESPACE getopt-for-windows::
            COMPONENT library)

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/getopt-for-windows
            COMPONENT headers
            FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/getopt-for-windows
            COMPONENT headers
            FILES_MATCHING PATTERN "*.hpp")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/getopt-for-windows
            COMPONENT headers
            FILES_MATCHING PATTERN "*.hxx")
endif()


set(CMAKE_PREFIX_PATH "${CMAKE_INSTALL_LIBDIR}/cmake/getopt-for-windows" ${CMAKE_PREFIX_PATH})

