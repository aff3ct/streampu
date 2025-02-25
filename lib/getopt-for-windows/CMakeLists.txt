# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------- CMAKE PROJECT
# ---------------------------------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0054 NEW)

project(getopt-for-windows LANGUAGES CXX C)

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------- CMAKE OPTIONS
# ---------------------------------------------------------------------------------------------------------------------
option(OPT_COMPILE_STATIC_LIB   "Compile the static library" ON)
option(OPT_COMPILE_SHARED_LIB   "Compile the shared library" ON)
option(OPT_STATUS_MESSAGE_QUIET "Suppress status messages"  OFF)

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------- CMAKE CONFIGURATION
# ---------------------------------------------------------------------------------------------------------------------
message(STATUS "getopt-for-windows - cmakelist")

# Include GNUInstallDirs to set standard installation directories
include(GNUInstallDirs)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Automatically export all symbols on Windows
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------- Modern Output Directories
# ---------------------------------------------------------------------------------------------------------------------
# Replace deprecated LIBRARY_OUTPUT_PATH with modern output directory variables.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# For multi-configuration generators (e.g., Visual Studio), specify configuration-specific directories:
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Debug")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release")

# ---------------------------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------------------- SOURCES
# ---------------------------------------------------------------------------------------------------------------------
# Retrieve all source files in the src directory
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------- OBJECTS/LIBS/EXE
# ---------------------------------------------------------------------------------------------------------------------
# Create an OBJECT library from the source files.
add_library(getopt-for-windows-obj OBJECT ${SRC_FILES})
set_target_properties(getopt-for-windows-obj PROPERTIES
                              POSITION_INDEPENDENT_CODE ON)  # Enables PIC

# Shared library
if(OPT_COMPILE_SHARED_LIB)
    add_library(getopt-for-windows-shared-lib SHARED $<TARGET_OBJECTS:getopt-for-windows-obj>)
    set_target_properties(getopt-for-windows-shared-lib PROPERTIES
                          OUTPUT_NAME getopt-for-windows
                          POSITION_INDEPENDENT_CODE ON)
    if(NOT OPT_STATUS_MESSAGE_QUIET)
        message(STATUS "getopt-for-windows - Compile: shared library")
    endif()
endif()

# Static library
if(OPT_COMPILE_STATIC_LIB)
    add_library(getopt-for-windows-static-lib STATIC $<TARGET_OBJECTS:getopt-for-windows-obj>)
    set_target_properties(getopt-for-windows-static-lib PROPERTIES
                          OUTPUT_NAME getopt-for-windows
                          POSITION_INDEPENDENT_CODE ON)
    if(NOT OPT_STATUS_MESSAGE_QUIET)
        message(STATUS "getopt-for-windows - Compile: static library")
    endif()
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------- HEADER-ONLY SETTINGS
# ---------------------------------------------------------------------------------------------------------------------
# Set include directories for the object target and its derivatives.
target_include_directories(getopt-for-windows-obj PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/getopt-for-windows/>)

if(OPT_COMPILE_SHARED_LIB)
    target_include_directories(getopt-for-windows-shared-lib PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/getopt-for-windows/>)
endif()

if(OPT_COMPILE_STATIC_LIB)
    target_include_directories(getopt-for-windows-static-lib PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/getopt-for-windows/>)
endif()

# ---------------------------------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------- COMPILER DEFINITIONS
# ---------------------------------------------------------------------------------------------------------------------
if(WIN32)
    set(WINDOWS_VISTA 0x0600)
    target_compile_definitions(getopt-for-windows-obj PUBLIC
        $<BUILD_INTERFACE:_WIN32_WINNT=${WINDOWS_VISTA}>
        $<INSTALL_INTERFACE:_WIN32_WINNT=${WINDOWS_VISTA}>)
    target_compile_definitions(getopt-for-windows-obj PUBLIC
        $<BUILD_INTERFACE:NOMINMAX>
        $<INSTALL_INTERFACE:NOMINMAX>)
endif()

# ---------------------------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------------------- EXPORT
# ---------------------------------------------------------------------------------------------------------------------
# The configuration file (getopt-for-windows-config.cmake) will be installed in
# ${CMAKE_INSTALL_LIBDIR}/cmake/getopt-for-windows/ and allows the end user
# to use find_package(getopt-for-windows) to locate this package.
set(GETOPT_EXPORT_TARGETS "")
if(OPT_COMPILE_SHARED_LIB)
    list(APPEND GETOPT_EXPORT_TARGETS getopt-for-windows-shared-lib)
endif()
if(OPT_COMPILE_STATIC_LIB)
    list(APPEND GETOPT_EXPORT_TARGETS getopt-for-windows-static-lib)
endif()

if(GETOPT_EXPORT_TARGETS)
    export(TARGETS ${GETOPT_EXPORT_TARGETS}
           NAMESPACE getopt-for-windows::
           FILE "${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/getopt-for-windows/getopt-for-windows-config.cmake")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------- INSTALL
# ---------------------------------------------------------------------------------------------------------------------
if(OPT_COMPILE_SHARED_LIB)
    install(TARGETS getopt-for-windows-shared-lib
            EXPORT getopt-for-windows-config
            RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
            COMPONENT library)
endif()

if(OPT_COMPILE_STATIC_LIB)
    install(TARGETS getopt-for-windows-static-lib
            EXPORT getopt-for-windows-config
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
            COMPONENT library)
endif()

install(EXPORT getopt-for-windows-config
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/getopt-for-windows/"
        NAMESPACE getopt-for-windows::
        COMPONENT library)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/getopt-for-windows
        COMPONENT headers
        FILES_MATCHING PATTERN "*.h")

# Optionally, add the installation package directory to CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH "${CMAKE_INSTALL_LIBDIR}/cmake/getopt-for-windows" ${CMAKE_PREFIX_PATH})
