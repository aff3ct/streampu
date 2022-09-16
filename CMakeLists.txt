# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------- CMAKE PROJECT
# ---------------------------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.2)
cmake_policy(SET CMP0054 NEW)

if(${CMAKE_VERSION} VERSION_EQUAL "3.14" OR ${CMAKE_VERSION} VERSION_GREATER "3.14")
    cmake_policy(SET CMP0083 NEW)
endif()

project(aff3ct-core)

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------- CMAKE OPTIONS
# ---------------------------------------------------------------------------------------------------------------------

option(AFF3CT_CORE_COMPILE_STATIC_LIB "Compile the static library"                             ON )
option(AFF3CT_CORE_COMPILE_SHARED_LIB "Compile the shared library"                             OFF)
option(AFF3CT_CORE_LINK_HWLOC         "Link with the hwloc library (used for threads pinning)" OFF)
option(AFF3CT_CORE_COLORS             "Enable the colors in the terminal"                      ON )

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git" AND NOT AFF3CT_CORE_OVERRIDE_VERSION)
    message(FATAL_ERROR "The '.git' folder can't be found, AFF3CT_CORE can't be compiled if it is not cloned "
                        "from a Git repository. Please do not download archives from GitHub and make a Git "
                        "clone instead (git clone https://github.com/aff3ct-core/aff3ct-core.git).")
endif()

if(NOT (WIN32 OR APPLE))
    option(AFF3CT_CORE_BACKTRACE "Enable the backtrace display when an exception is raised" ON)
else()
    set(AFF3CT_CORE_BACKTRACE OFF)
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------- CMAKE CONFIGURATION
# ---------------------------------------------------------------------------------------------------------------------

# set CMAKE_INSTALL_BINDIR, CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_INCLUDEDIR and CMAKE_INSTALL_DATAROOTDIR variables
include(GNUInstallDirs)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_MACOSX_RPATH 1)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify bin and lib paths
set(EXECUTABLE_OUTPUT_PATH bin/)
set(LIBRARY_OUTPUT_PATH lib/)

# Generate the source files list
file(GLOB_RECURSE source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------ CREATE FILTERS
# ---------------------------------------------------------------------------------------------------------------------

# Filters creation for IDEs (tested on Visual Studio and based on the "source_group" function)
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if(IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

assign_source_group(${source_files})

# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------- OBJECTS/LIBS/EXE
# ---------------------------------------------------------------------------------------------------------------------

if(${CMAKE_VERSION} VERSION_EQUAL "3.14" OR ${CMAKE_VERSION} VERSION_GREATER "3.14")
    include(CheckPIESupported)
    check_pie_supported()
endif()

add_library(aff3ct-core-obj OBJECT ${source_files})
set_target_properties(aff3ct-core-obj PROPERTIES
                                      POSITION_INDEPENDENT_CODE ON) # set -fpic

# Library
if(AFF3CT_CORE_COMPILE_SHARED_LIB)
    add_library(aff3ct-core-shared-lib SHARED $<TARGET_OBJECTS:aff3ct-core-obj>)
    set_target_properties(aff3ct-core-shared-lib PROPERTIES
                                                 OUTPUT_NAME aff3ct-core
                                                 POSITION_INDEPENDENT_CODE ON) # set -fpic
    message(STATUS "AFF3CT_CORE - Compile: shared library")
endif(AFF3CT_CORE_COMPILE_SHARED_LIB)
if(AFF3CT_CORE_COMPILE_STATIC_LIB)
    add_library(aff3ct-core-static-lib STATIC $<TARGET_OBJECTS:aff3ct-core-obj>)
    set_target_properties(aff3ct-core-static-lib PROPERTIES
                                                 OUTPUT_NAME aff3ct-core
                                                 POSITION_INDEPENDENT_CODE ON) # set -fpic
    message(STATUS "AFF3CT_CORE - Compile: static library")
endif(AFF3CT_CORE_COMPILE_STATIC_LIB)

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------ COMPILER DEFINITIONS
# ---------------------------------------------------------------------------------------------------------------------

macro(aff3ct_core_target_compile_definitions privacy def)
    if(AFF3CT_CORE_COMPILE_SHARED_LIB)
        target_compile_definitions(aff3ct-core-shared-lib ${privacy} $<BUILD_INTERFACE:${def}> $<INSTALL_INTERFACE:${def}>)
    endif()
    if(AFF3CT_CORE_COMPILE_STATIC_LIB)
        target_compile_definitions(aff3ct-core-static-lib ${privacy} $<BUILD_INTERFACE:${def}> $<INSTALL_INTERFACE:${def}>)
    endif()
endmacro()

# by system
if(WIN32) # for Windows operating system in general
    set(WINDOWS_VISTA 0x0600)
    aff3ct_core_target_compile_definitions(PUBLIC _WIN32_WINNT=${WINDOWS_VISTA})
    aff3ct_core_target_compile_definitions(PUBLIC NOMINMAX)
    message(STATUS "AFF3CT-core - System: Windows")
elseif(APPLE) # for macOS
    message(STATUS "AFF3CT-core - System: macOS")
elseif(UNIX AND NOT APPLE) # for Linux, BSD, Solaris, Minix
    message(STATUS "AFF3CT-core - System: Unix/Linux")
endif()

# common
if(AFF3CT_CORE_COLORS)
    aff3ct_core_target_compile_definitions(PUBLIC AFF3CT_CORE_COLORS)
    message(STATUS "AFF3CT-core - Terminal colors: on")
else()
    message(STATUS "AFF3CT-core - Terminal colors: off")
endif()
if(AFF3CT_CORE_BACKTRACE)
    aff3ct_core_target_compile_definitions(PUBLIC AFF3CT_CORE_BACKTRACE)
    message(STATUS "AFF3CT-core - Backtrace: on")
else()
    message(STATUS "AFF3CT-core - Backtrace: off")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------- HEADER ONLY LIBRARIES
# ---------------------------------------------------------------------------------------------------------------------

macro(aff3ct_core_target_include_directories privacy dir_build dir_install)
    target_include_directories(aff3ct-core-obj ${privacy}
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir_build}/>
                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/aff3ct-core/${dir_install}>)

    if(AFF3CT_CORE_COMPILE_SHARED_LIB)
        target_include_directories(aff3ct-core-shared-lib ${privacy}
                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir_build}/>
                                   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/aff3ct-core/${dir_install}>)
    endif()
    if(AFF3CT_CORE_COMPILE_STATIC_LIB)
        target_include_directories(aff3ct-core-static-lib ${privacy}
                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir_build}/>
                                   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/aff3ct-core/${dir_install}>)
    endif()
endmacro()

macro(aff3ct_core_target_include_directories2 privacy dir)
    if(AFF3CT_CORE_COMPILE_SHARED_LIB)
        target_include_directories(aff3ct-core-shared-lib ${privacy} $<BUILD_INTERFACE:${dir}> $<INSTALL_INTERFACE:${dir}>)
    endif()
    if(AFF3CT_CORE_COMPILE_STATIC_LIB)
        target_include_directories(aff3ct-core-static-lib ${privacy} $<BUILD_INTERFACE:${dir}> $<INSTALL_INTERFACE:${dir}>)
    endif()
endmacro()

# AFF3CT_CORE headers
aff3ct_core_target_include_directories(PRIVATE "src"     "aff3ct-core")
aff3ct_core_target_include_directories(PUBLIC  "include" "aff3ct-core")

# rang
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/rang/include/rang.hpp")
    aff3ct_core_target_include_directories(PUBLIC "lib/rang/include" "rang")
    message(STATUS "AFF3CT-core - Header found: rang")
else()
    message(FATAL_ERROR "AFF3CT-core - rang can't be found, try to init the submodule with the following cmd:\n"
                        "$ git submodule update --init -- ../lib/rang/")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------- COMPILED LIBRARIES
# ---------------------------------------------------------------------------------------------------------------------

macro(aff3ct_core_target_link_libraries privacy lib)
    if(AFF3CT_CORE_COMPILE_SHARED_LIB)
       target_link_libraries(aff3ct-core-shared-lib ${privacy} ${lib})
    endif(AFF3CT_CORE_COMPILE_SHARED_LIB)
    if(AFF3CT_CORE_COMPILE_STATIC_LIB)
        target_link_libraries(aff3ct-core-static-lib ${privacy} ${lib})
    endif(AFF3CT_CORE_COMPILE_STATIC_LIB)
endmacro()

# hwloc
if(AFF3CT_CORE_LINK_HWLOC)
    aff3ct_core_target_compile_definitions(PUBLIC "AFF3CT_CORE_HWLOC")

    find_package(Hwloc REQUIRED QUIET)
    if(Hwloc_FOUND)
        message(STATUS "AFF3CT-core - Library found: hwloc")
        aff3ct_core_target_include_directories2(PUBLIC "${Hwloc_INCLUDE_DIRS}")
        aff3ct_core_target_link_libraries(PUBLIC "${Hwloc_LIBRARIES}")
    endif(Hwloc_FOUND)
endif(AFF3CT_CORE_LINK_HWLOC)

# Threads
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
aff3ct_core_target_link_libraries(PUBLIC Threads::Threads)

# ---------------------------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------------------- EXPORT
# ---------------------------------------------------------------------------------------------------------------------

if (AFF3CT_CORE_COMPILE_SHARED_LIB AND NOT AFF3CT_CORE_COMPILE_STATIC_LIB)
    export(TARGETS
           aff3ct-core-shared-lib
           NAMESPACE aff3ct-core::
           FILE "lib/cmake/aff3ct-core/aff3ct-core-config.cmake")
endif()

if (AFF3CT_CORE_COMPILE_STATIC_LIB AND NOT AFF3CT_CORE_COMPILE_SHARED_LIB)
    export(TARGETS
           aff3ct-core-static-lib
           NAMESPACE aff3ct-core::
           FILE "lib/cmake/aff3ct-core/aff3ct-core-config.cmake")
endif()

if(AFF3CT_CORE_COMPILE_SHARED_LIB AND AFF3CT_CORE_COMPILE_STATIC_LIB)
    export(TARGETS
           aff3ct-core-shared-lib
           aff3ct-core-static-lib
           NAMESPACE aff3ct-core::
           FILE "lib/cmake/aff3ct-core/aff3ct-core-config.cmake")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------- INSTALL
# ---------------------------------------------------------------------------------------------------------------------

if(AFF3CT_CORE_COMPILE_SHARED_LIB)
    if(WIN32)
        install(TARGETS aff3ct-core-shared-lib
                EXPORT aff3ct-core-config
                RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
                COMPONENT library)
    else()
        install(TARGETS aff3ct-core-shared-lib
                EXPORT aff3ct-core-config
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
                COMPONENT library)
    endif()
endif()
if(AFF3CT_CORE_COMPILE_STATIC_LIB)
    install(TARGETS aff3ct-core-static-lib
            EXPORT aff3ct-core-config
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
            COMPONENT library)
endif()

if (AFF3CT_CORE_COMPILE_SHARED_LIB OR AFF3CT_CORE_COMPILE_STATIC_LIB)
    install(EXPORT
            aff3ct-core-config
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/aff3ct-core/"
            NAMESPACE aff3ct-core::
            COMPONENT library)

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/aff3ct-core/aff3ct-core-config-version.cmake"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/aff3ct-core/"
            COMPONENT library)

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/aff3ct-core/aff3ct-core
            COMPONENT headers
            FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/aff3ct-core/aff3ct-core
            COMPONENT headers
            FILES_MATCHING PATTERN "*.hpp")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/aff3ct-core/aff3ct-core
            COMPONENT headers
            FILES_MATCHING PATTERN "*.hxx")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/rang/include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/aff3ct-core/rang
            COMPONENT headers
            FILES_MATCHING PATTERN "*.hpp")
endif()
