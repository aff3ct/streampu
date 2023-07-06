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
option(AFF3CT_CORE_TESTS              "Enable the compilation of the tests"                    ON )

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
list(APPEND aff3ct_core_targets_list aff3ct-core-obj)

# Library
if(AFF3CT_CORE_COMPILE_SHARED_LIB)
    add_library(aff3ct-core-shared-lib SHARED $<TARGET_OBJECTS:aff3ct-core-obj>)
    set_target_properties(aff3ct-core-shared-lib PROPERTIES
                                                 OUTPUT_NAME aff3ct-core
                                                 POSITION_INDEPENDENT_CODE ON) # set -fpic
    list(APPEND aff3ct_core_targets_list aff3ct-core-shared-lib)
    message(STATUS "AFF3CT-core - Compile: shared library")
endif(AFF3CT_CORE_COMPILE_SHARED_LIB)
if(AFF3CT_CORE_COMPILE_STATIC_LIB)
    add_library(aff3ct-core-static-lib STATIC $<TARGET_OBJECTS:aff3ct-core-obj>)
    set_target_properties(aff3ct-core-static-lib PROPERTIES
                                                 OUTPUT_NAME aff3ct-core
                                                 POSITION_INDEPENDENT_CODE ON) # set -fpic
    list(APPEND aff3ct_core_targets_list aff3ct-core-static-lib)
    message(STATUS "AFF3CT-core - Compile: static library")
endif(AFF3CT_CORE_COMPILE_STATIC_LIB)

# Tests
if(AFF3CT_CORE_TESTS)
    add_executable(aff3ct-core-test-simple-chain $<TARGET_OBJECTS:aff3ct-core-obj>
                                                 ${CMAKE_CURRENT_SOURCE_DIR}/tests/simple_chain.cpp)
    set_target_properties(aff3ct-core-test-simple-chain PROPERTIES
                                                        OUTPUT_NAME test-simple-chain
                                                        POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-simple-chain)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-simple-chain)
    ###=========================================================================================###
    #Ajout de la simple-chain-test 
    add_executable(aff3ct-core-test-simple-chain-io $<TARGET_OBJECTS:aff3ct-core-obj>
                                                 ${CMAKE_CURRENT_SOURCE_DIR}/tests/simple_chain_io.cpp)
    set_target_properties(aff3ct-core-test-simple-chain-io PROPERTIES
                                                        OUTPUT_NAME test-simple-chain-io
                                                        POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-simple-chain-io)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-simple-chain-io)
    ###=========================================================================================###
    ###=========================================================================================###
    #Ajout de la simple-chain-hybrid-test 
    add_executable(aff3ct-core-test-simple-chain-hybride $<TARGET_OBJECTS:aff3ct-core-obj>
                                                 ${CMAKE_CURRENT_SOURCE_DIR}/tests/simple_chain_hybride.cpp)
    set_target_properties(aff3ct-core-test-simple-chain-hybride PROPERTIES
                                                        OUTPUT_NAME test-simple-chain-hybride
                                                        POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-simple-chain-hybride)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-simple-chain-hybride)
    ###=========================================================================================###
    add_executable(aff3ct-core-test-for-loop $<TARGET_OBJECTS:aff3ct-core-obj>
                                                 ${CMAKE_CURRENT_SOURCE_DIR}/tests/for_loop.cpp)
    set_target_properties(aff3ct-core-test-for-loop PROPERTIES
                                                    OUTPUT_NAME test-for-loop
                                                    POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-for-loop)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-for-loop)

    add_executable(aff3ct-core-test-do-while-loop $<TARGET_OBJECTS:aff3ct-core-obj>
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/tests/do_while_loop.cpp)
    set_target_properties(aff3ct-core-test-do-while-loop PROPERTIES
                                                         OUTPUT_NAME test-do-while-loop
                                                         POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-do-while-loop)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-do-while-loop)

    add_executable(aff3ct-core-test-exclusive-paths $<TARGET_OBJECTS:aff3ct-core-obj>
                                                    ${CMAKE_CURRENT_SOURCE_DIR}/tests/exclusive_paths.cpp)
    set_target_properties(aff3ct-core-test-exclusive-paths PROPERTIES
                                                           OUTPUT_NAME test-exclusive-paths
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-exclusive-paths)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-exclusive-paths)

    add_executable(aff3ct-core-test-nested-loops $<TARGET_OBJECTS:aff3ct-core-obj>
                                                 ${CMAKE_CURRENT_SOURCE_DIR}/tests/nested_loops.cpp)
    set_target_properties(aff3ct-core-test-nested-loops PROPERTIES
                                                        OUTPUT_NAME test-nested-loops
                                                        POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-nested-loops)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-nested-loops)

    add_executable(aff3ct-core-test-simple-pipeline $<TARGET_OBJECTS:aff3ct-core-obj>
                                                    ${CMAKE_CURRENT_SOURCE_DIR}/tests/simple_pipeline.cpp)
    set_target_properties(aff3ct-core-test-simple-pipeline PROPERTIES
                                                           OUTPUT_NAME test-simple-pipeline
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-simple-pipeline)
    ###======================================================================================================###
    add_executable(aff3ct-core-test-simple-pipeline_io $<TARGET_OBJECTS:aff3ct-core-obj>
                                                    ${CMAKE_CURRENT_SOURCE_DIR}/tests/simple_pipeline_io.cpp)
    set_target_properties(aff3ct-core-test-simple-pipeline_io PROPERTIES
                                                           OUTPUT_NAME test-simple-pipeline_io
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-simple-pipeline_io)
    ###======================================================================================================###
    # Ajout du code pour pipeline_full_io
    ###======================================================================================================###
    add_executable(aff3ct-core-test-simple-pipeline_full_io $<TARGET_OBJECTS:aff3ct-core-obj>
                                                    ${CMAKE_CURRENT_SOURCE_DIR}/tests/simple_pipeline_full_io.cpp)
    set_target_properties(aff3ct-core-test-simple-pipeline_full_io PROPERTIES
                                                           OUTPUT_NAME test-simple-pipeline_full_io
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-simple-pipeline_full_io)
    ###======================================================================================================###
     # Ajout du code pour pipeline_multiple_stage
    ###======================================================================================================###
    add_executable(aff3ct-core-test-simple-pipeline_multiple_stage $<TARGET_OBJECTS:aff3ct-core-obj>
                                                    ${CMAKE_CURRENT_SOURCE_DIR}/tests/simple_pipeline_multiple_stage_io.cpp)
    set_target_properties(aff3ct-core-test-simple-pipeline_multiple_stage PROPERTIES
                                                           OUTPUT_NAME test-simple-pipeline_multiple_stage
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie 
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-simple-pipeline_multiple_stage)
    ###======================================================================================================###

     ###======================================================================================================###
     # Ajout du code pour pipeline_inter_stage
    ###======================================================================================================###
    add_executable(aff3ct-core-test-complexe-pipeline_inter_stage $<TARGET_OBJECTS:aff3ct-core-obj>
                                                    ${CMAKE_CURRENT_SOURCE_DIR}/tests/complexe_pipeline_inter_stage.cpp)
    set_target_properties(aff3ct-core-test-complexe-pipeline_inter_stage PROPERTIES
                                                           OUTPUT_NAME test-complexe-pipeline_inter_stage
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-complexe-pipeline_inter_stage)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-complexe-pipeline_inter_stage)
    ###======================================================================================================###

     ###======================================================================================================###
     # Ajout du code pour pipeline_double_chain
    ###======================================================================================================###
    add_executable(aff3ct-core-test-simple-pipeline_double_chain $<TARGET_OBJECTS:aff3ct-core-obj>
                                                    ${CMAKE_CURRENT_SOURCE_DIR}/tests/pipeline_double_chain.cpp)
    set_target_properties(aff3ct-core-test-simple-pipeline_double_chain PROPERTIES
                                                           OUTPUT_NAME test-simple-pipeline_double_chain
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-simple-pipeline_double_chain)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-simple-pipeline_double_chain)
    ###======================================================================================================###

endif()

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------ COMPILER DEFINITIONS
# ---------------------------------------------------------------------------------------------------------------------

macro(aff3ct_core_target_compile_definitions targets privacy def)
    foreach(_target IN ITEMS ${targets})
        target_compile_definitions(${_target} ${privacy} $<BUILD_INTERFACE:${def}> $<INSTALL_INTERFACE:${def}>)
    endforeach()
endmacro()

# by system
if(WIN32) # for Windows operating system in general
    set(WINDOWS_VISTA 0x0600)
    aff3ct_core_target_compile_definitions("${aff3ct_core_targets_list}" PUBLIC _WIN32_WINNT=${WINDOWS_VISTA})
    aff3ct_core_target_compile_definitions("${aff3ct_core_targets_list}" PUBLIC NOMINMAX)
    message(STATUS "AFF3CT-core - System: Windows")
elseif(APPLE) # for macOS
    message(STATUS "AFF3CT-core - System: macOS")
elseif(UNIX AND NOT APPLE) # for Linux, BSD, Solaris, Minix
    message(STATUS "AFF3CT-core - System: Unix/Linux")
endif()

# common
if(AFF3CT_CORE_COLORS)
    aff3ct_core_target_compile_definitions("${aff3ct_core_targets_list}" PUBLIC AFF3CT_CORE_COLORS)
    message(STATUS "AFF3CT-core - Terminal colors: on")
else()
    message(STATUS "AFF3CT-core - Terminal colors: off")
endif()
if(AFF3CT_CORE_BACKTRACE)
    aff3ct_core_target_compile_definitions("${aff3ct_core_targets_list}" PUBLIC AFF3CT_CORE_BACKTRACE)
    message(STATUS "AFF3CT-core - Backtrace: on")
else()
    message(STATUS "AFF3CT-core - Backtrace: off")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------- HEADER ONLY LIBRARIES
# ---------------------------------------------------------------------------------------------------------------------

macro(aff3ct_core_target_include_directories targets privacy dir_build dir_install)
    foreach(_target IN ITEMS ${targets})
        target_include_directories(${_target}
                                   ${privacy}
                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir_build}/>
                                   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/aff3ct-core/${dir_install}>)
    endforeach()
endmacro()

macro(aff3ct_core_target_include_directories2 targets privacy dir)
    foreach(_target IN ITEMS ${targets})
        target_include_directories(${_target} ${privacy} $<BUILD_INTERFACE:${dir}> $<INSTALL_INTERFACE:${dir}>)
    endforeach()
endmacro()

# AFF3CT_CORE headers
aff3ct_core_target_include_directories("${aff3ct_core_targets_list}" PRIVATE "src"     "aff3ct-core")
aff3ct_core_target_include_directories("${aff3ct_core_targets_list}" PUBLIC  "include" "aff3ct-core")

# rang
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/rang/include/rang.hpp")
    aff3ct_core_target_include_directories("${aff3ct_core_targets_list}" PUBLIC "lib/rang/include" "rang")
    message(STATUS "AFF3CT-core - Header found: rang")
else()
    message(FATAL_ERROR "AFF3CT-core - rang can't be found, try to init the submodule with the following cmd:\n"
                        "$ git submodule update --init -- ../lib/rang/")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------- COMPILED LIBRARIES
# ---------------------------------------------------------------------------------------------------------------------

macro(aff3ct_core_target_link_libraries targets privacy lib)
    foreach(_target IN ITEMS ${targets})
        target_link_libraries(${_target} ${privacy} ${lib})
    endforeach()
endmacro()

# hwloc
if(AFF3CT_CORE_LINK_HWLOC)
    aff3ct_core_target_compile_definitions("${aff3ct_core_targets_list}" PUBLIC "AFF3CT_CORE_HWLOC")

    find_package(Hwloc REQUIRED QUIET)
    if(Hwloc_FOUND)
        message(STATUS "AFF3CT-core - Library found: hwloc")
        aff3ct_core_target_include_directories2("${aff3ct_core_targets_list}" PUBLIC "${Hwloc_INCLUDE_DIRS}")
        aff3ct_core_target_link_libraries("${aff3ct_core_targets_list}" PUBLIC "${Hwloc_LIBRARIES}")
    endif(Hwloc_FOUND)
endif(AFF3CT_CORE_LINK_HWLOC)

# Threads
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
aff3ct_core_target_link_libraries("${aff3ct_core_targets_list}" PUBLIC Threads::Threads)

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

# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------------------- TEST
# ---------------------------------------------------------------------------------------------------------------------

if(AFF3CT_CORE_TESTS)
    enable_testing()

    macro(aff3ct_core_run_1t_tests targets)
        foreach(_target IN ITEMS ${targets})
             add_test(NAME run-1t::${_target} COMMAND ${_target} -e 100 -t 1)
        endforeach()
    endmacro()

    aff3ct_core_run_1t_tests("${aff3ct_core_tests_list}")

    macro(aff3ct_core_run_tests targets)
        foreach(_target IN ITEMS ${targets})
             add_test(NAME run::${_target} COMMAND ${_target} -e 100)
        endforeach()
    endmacro()

    aff3ct_core_run_tests("${aff3ct_core_tests_list}")

    macro(aff3ct_core_inter_frames_tests targets)
        foreach(_target IN ITEMS ${targets})
             add_test(NAME inter-frames::${_target} COMMAND ${_target} -e 100 -f 13)
        endforeach()
    endmacro()

    aff3ct_core_inter_frames_tests("${aff3ct_core_tests_list}")

    macro(aff3ct_core_step_by_step_tests targets)
        foreach(_target IN ITEMS ${targets})
             add_test(NAME step-by-step::${_target} COMMAND ${_target} -e 51 -f 13 -b)
        endforeach()
    endmacro()

    aff3ct_core_step_by_step_tests("${aff3ct_core_tests_list}")

    macro(aff3ct_core_copy_mode_tests targets)
        foreach(_target IN ITEMS ${targets})
             add_test(NAME copy-mode::${_target} COMMAND ${_target} -e 142 -f 13 -c)
        endforeach()
    endmacro()

    aff3ct_core_copy_mode_tests("${aff3ct_core_tests_list}")

    macro(aff3ct_core_graph_dot_tests targets)
        foreach(_target IN ITEMS ${targets})
             add_test(NAME graph-dot::${_target} COMMAND ${_target} -e 142 -f 13 -o graph.dot)
        endforeach()
    endmacro()

    aff3ct_core_graph_dot_tests("${aff3ct_core_tests_list}")

    add_test(NAME subseq0::aff3ct-core-test-simple-chain COMMAND aff3ct-core-test-simple-chain -e 142 -f 13 -u)
    add_test(NAME subseq1::aff3ct-core-test-simple-chain COMMAND aff3ct-core-test-simple-chain -v)

    add_test(NAME path0::aff3ct-core-test-exclusive-paths COMMAND aff3ct-core-test-exclusive-paths -e 142 -f 13 -a 0)
    add_test(NAME path1::aff3ct-core-test-exclusive-paths COMMAND aff3ct-core-test-exclusive-paths -e 142 -f 13 -a 1)
    add_test(NAME path2::aff3ct-core-test-exclusive-paths COMMAND aff3ct-core-test-exclusive-paths -e 142 -f 13 -a 2)
    add_test(NAME cyclic::aff3ct-core-test-exclusive-paths COMMAND aff3ct-core-test-exclusive-paths -e 142 -f 13 -y)

    add_test(NAME sequence0::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -q -t 1 -i ./bin/test-simple-pipeline)
    add_test(NAME sequence1::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -q -t 1 -i ./bin/test-simple-pipeline -p -g)
    add_test(NAME sequence2::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -q -t 1 -i ./bin/test-simple-pipeline -b)
    add_test(NAME sequence3::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -q -t 1 -i ./bin/test-simple-pipeline -o graph.dot)
    add_test(NAME pipeline0::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -i ./bin/test-simple-pipeline)
    add_test(NAME pipeline1::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -i ./bin/test-simple-pipeline -p)
    add_test(NAME pipeline2::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -i ./bin/test-simple-pipeline -w)
    add_test(NAME pipeline3::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -i ./bin/test-simple-pipeline -u 1)
    add_test(NAME pipeline4::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -i ./bin/test-simple-pipeline -u 1024)
    add_test(NAME pipeline5::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -i ./bin/test-simple-pipeline -u 128 -o graph.dot)
endif()
