# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------- CMAKE PROJECT
# ---------------------------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0054 NEW)

if(${CMAKE_VERSION} VERSION_EQUAL "3.14" OR ${CMAKE_VERSION} VERSION_GREATER "3.14")
    cmake_policy(SET CMP0083 NEW)
endif()

project(aff3ct-core CXX)

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
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/simple_chain.cpp)
    set_target_properties(aff3ct-core-test-simple-chain PROPERTIES
                                                        OUTPUT_NAME test-simple-chain
                                                        POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-simple-chain)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-simple-chain)

    add_executable(aff3ct-core-test-simple-chain-fwd $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/simple_chain_fwd.cpp)
    set_target_properties(aff3ct-core-test-simple-chain-fwd PROPERTIES
                                                        OUTPUT_NAME test-simple-chain-fwd
                                                        POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-simple-chain-fwd)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-simple-chain-fwd)

    add_executable(aff3ct-core-test-simple-chain-hybrid $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/simple_chain_hybrid.cpp)
    set_target_properties(aff3ct-core-test-simple-chain-hybrid PROPERTIES
                                                        OUTPUT_NAME test-simple-chain-hybrid
                                                        POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-simple-chain-hybrid)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-simple-chain-hybrid)

    add_executable(aff3ct-core-test-for-loop $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/for_loop.cpp)
    set_target_properties(aff3ct-core-test-for-loop PROPERTIES
                                                    OUTPUT_NAME test-for-loop
                                                    POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-for-loop)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-for-loop)

    add_executable(aff3ct-core-test-do-while-loop $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/do_while_loop.cpp)
    set_target_properties(aff3ct-core-test-do-while-loop PROPERTIES
                                                         OUTPUT_NAME test-do-while-loop
                                                         POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-do-while-loop)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-do-while-loop)

    add_executable(aff3ct-core-test-exclusive-paths $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/exclusive_paths.cpp)
    set_target_properties(aff3ct-core-test-exclusive-paths PROPERTIES
                                                           OUTPUT_NAME test-exclusive-paths
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-exclusive-paths)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-exclusive-paths)

    add_executable(aff3ct-core-test-nested-loops $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/nested_loops.cpp)
    set_target_properties(aff3ct-core-test-nested-loops PROPERTIES
                                                        OUTPUT_NAME test-nested-loops
                                                        POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_tests_list aff3ct-core-test-nested-loops)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-nested-loops)

    add_executable(aff3ct-core-test-simple-pipeline $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/simple_pipeline.cpp)
    set_target_properties(aff3ct-core-test-simple-pipeline PROPERTIES
                                                           OUTPUT_NAME test-simple-pipeline
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-simple-pipeline)

    add_executable(aff3ct-core-test-complex-pipeline-inter-stage $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced/complex_pipeline_inter_stage.cpp)
    set_target_properties(aff3ct-core-test-complex-pipeline-inter-stage PROPERTIES
                                                           OUTPUT_NAME test-complex-pipeline-inter-stage
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-complex-pipeline-inter-stage)

    add_executable(aff3ct-core-test-pipeline-double-chain $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced/pipeline_double_chain.cpp)
    set_target_properties(aff3ct-core-test-pipeline-double-chain PROPERTIES
                                                           OUTPUT_NAME test-pipeline-double-chain
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-pipeline-double-chain)

    add_executable(aff3ct-core-test-complex-pipeline-full-fwd $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced/complex_pipeline_full_fwd.cpp)
    set_target_properties(aff3ct-core-test-complex-pipeline-full-fwd PROPERTIES
                                                           OUTPUT_NAME test-complex-pipeline-full-fwd
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-complex-pipeline-full-fwd)

    add_executable(aff3ct-core-test-complex-pipeline-mix-fwd $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced/complex_pipeline_mix_fwd.cpp)
    set_target_properties(aff3ct-core-test-complex-pipeline-mix-fwd PROPERTIES
                                                           OUTPUT_NAME test-complex-pipeline-mix-fwd
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-complex-pipeline-mix-fwd)

    add_executable(aff3ct-core-test-generic-pipeline $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced/generic_pipeline.cpp)
    set_target_properties(aff3ct-core-test-generic-pipeline PROPERTIES
                                                           OUTPUT_NAME test-generic-pipeline
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-generic-pipeline)

    add_executable(aff3ct-core-test-exclusive-paths-pipeline $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/ctrl_flw/exclusive_paths_pipeline.cpp)
    set_target_properties(aff3ct-core-test-exclusive-paths-pipeline PROPERTIES
                                                           OUTPUT_NAME test-exclusive-paths-pipeline
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    #list(APPEND aff3ct_core_tests_list aff3ct-core-test-nest-loops-pipeline)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-exclusive-paths-pipeline)

    add_executable(aff3ct-core-test-nest-loops-pipeline $<TARGET_OBJECTS:aff3ct-core-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/ctrl_flw/nested_loops_pipeline.cpp)
    set_target_properties(aff3ct-core-test-nest-loops-pipeline PROPERTIES
                                                           OUTPUT_NAME test-nested-loops-pipeline
                                                           POSITION_INDEPENDENT_CODE ON) # set -fpie
    # list(APPEND aff3ct_core_tests_list aff3ct-core-test-nest-loops-pipeline)
    list(APPEND aff3ct_core_targets_list aff3ct-core-test-nest-loops-pipeline)

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
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full")
    include(CTest)
    enable_testing()

    macro(aff3ct_core_run_1t_tests targets)
        foreach(_target IN ITEMS ${targets})
            add_test(NAME run-1t::${_target} COMMAND ${_target} -d 2048 -s 5 -e 10 -t 1)
            set_tests_properties(run-1t::${_target} PROPERTIES LABELS run-1t)
        endforeach()
    endmacro()

    aff3ct_core_run_1t_tests("${aff3ct_core_tests_list}")

    macro(aff3ct_core_run_tests targets)
        foreach(_target IN ITEMS ${targets})
            add_test(NAME run::${_target} COMMAND ${_target} -d 2048 -s 5 -e 100)
            set_tests_properties(run::${_target} PROPERTIES LABELS run)
        endforeach()
    endmacro()

    aff3ct_core_run_tests("${aff3ct_core_tests_list}")

    macro(aff3ct_core_inter_frames_tests targets)
        foreach(_target IN ITEMS ${targets})
            add_test(NAME inter-frames::${_target} COMMAND ${_target} -d 2048 -s 5 -e 10 -f 13)
            set_tests_properties(inter-frames::${_target} PROPERTIES LABELS inter-frames)
        endforeach()
    endmacro()

    aff3ct_core_inter_frames_tests("${aff3ct_core_tests_list}")

    macro(aff3ct_core_step_by_step_tests targets)
        foreach(_target IN ITEMS ${targets})
            add_test(NAME step-by-step::${_target} COMMAND ${_target} -d 2048 -s 5 -e 11 -f 13 -b)
            set_tests_properties(step-by-step::${_target} PROPERTIES LABELS step-by-step)
        endforeach()
    endmacro()

    aff3ct_core_step_by_step_tests("${aff3ct_core_tests_list}")

    macro(aff3ct_core_copy_mode_tests targets)
        foreach(_target IN ITEMS ${targets})
            add_test(NAME copy-mode::${_target} COMMAND ${_target} -d 2048 -s 5 -e 15 -f 13 -c)
            set_tests_properties(copy-mode::${_target} PROPERTIES LABELS copy-mode)
        endforeach()
    endmacro()

    aff3ct_core_copy_mode_tests("${aff3ct_core_tests_list}")

    macro(aff3ct_core_graph_dot_tests targets)
        foreach(_target IN ITEMS ${targets})
            add_test(NAME graph-dot::${_target} COMMAND ${_target} -d 2048 -s 5 -e 15 -f 13 -o graph.dot)
            set_tests_properties(graph-dot::${_target} PROPERTIES LABELS graph-dot)
        endforeach()
    endmacro()

    aff3ct_core_graph_dot_tests("${aff3ct_core_tests_list}")

    set(INPUT_FILE "../mkdocs.yml")

    add_test(NAME subseq0::aff3ct-core-test-simple-chain COMMAND aff3ct-core-test-simple-chain -d 2048 -s 5 -e 15 -f 13 -u)
    set_tests_properties(subseq0::aff3ct-core-test-simple-chain PROPERTIES LABELS subseq)
    add_test(NAME subseq1::aff3ct-core-test-simple-chain COMMAND aff3ct-core-test-simple-chain -d 2048 -s 5 -v)
    set_tests_properties(subseq1::aff3ct-core-test-simple-chain PROPERTIES LABELS subseq)

    add_test(NAME path0::aff3ct-core-test-exclusive-paths COMMAND aff3ct-core-test-exclusive-paths -d 2048 -s 5 -e 10 -f 13 -a 0)
    set_tests_properties(path0::aff3ct-core-test-exclusive-paths PROPERTIES LABELS exclusive-paths)
    add_test(NAME path1::aff3ct-core-test-exclusive-paths COMMAND aff3ct-core-test-exclusive-paths -d 2048 -s 5 -e 10 -f 13 -a 1)
    set_tests_properties(path1::aff3ct-core-test-exclusive-paths PROPERTIES LABELS exclusive-paths)
    add_test(NAME path2::aff3ct-core-test-exclusive-paths COMMAND aff3ct-core-test-exclusive-paths -d 2048 -s 5 -e 10 -f 13 -a 2)
    set_tests_properties(path2::aff3ct-core-test-exclusive-paths PROPERTIES LABELS exclusive-paths)
    add_test(NAME cyclic::aff3ct-core-test-exclusive-paths COMMAND aff3ct-core-test-exclusive-paths -d 2048 -s 5 -e 10 -f 13 -y)
    set_tests_properties(cyclic::aff3ct-core-test-exclusive-paths PROPERTIES LABELS exclusive-paths)

    add_test(NAME sequence0::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -q -t 1 -i ${INPUT_FILE})
    set_tests_properties(sequence0::aff3ct-core-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME sequence1::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -q -t 1 -i ${INPUT_FILE} -p -g)
    set_tests_properties(sequence1::aff3ct-core-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME sequence2::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -q -t 1 -i ${INPUT_FILE} -b)
    set_tests_properties(sequence2::aff3ct-core-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME sequence3::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -q -t 1 -i ${INPUT_FILE} -o graph.dot)
    set_tests_properties(sequence3::aff3ct-core-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline0::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -i ${INPUT_FILE})
    set_tests_properties(pipeline0::aff3ct-core-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline1::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -i ${INPUT_FILE} -p)
    set_tests_properties(pipeline1::aff3ct-core-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline2::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -i ${INPUT_FILE} -w)
    set_tests_properties(pipeline2::aff3ct-core-test-simple-pipeline PROPERTIES LABELS "simple-pipeline;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(
    add_test(NAME pipeline3::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -i ${INPUT_FILE} -u 1)
    set_tests_properties(pipeline3::aff3ct-core-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline4::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -i ${INPUT_FILE} -u 1024)
    set_tests_properties(pipeline4::aff3ct-core-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline5::aff3ct-core-test-simple-pipeline COMMAND aff3ct-core-test-simple-pipeline -i ${INPUT_FILE} -u 128 -o graph.dot)
    set_tests_properties(pipeline5::aff3ct-core-test-simple-pipeline PROPERTIES LABELS simple-pipeline)

    # complex pipeline interstage
    add_test(NAME pipeline0::aff3ct-core-test-complex-pipeline-inter-stage COMMAND aff3ct-core-test-complex-pipeline-inter-stage -i ${INPUT_FILE})
    set_tests_properties(pipeline0::aff3ct-core-test-complex-pipeline-inter-stage PROPERTIES LABELS complex-pipeline-inter-stage)
    add_test(NAME pipeline1::aff3ct-core-test-complex-pipeline-inter-stage COMMAND aff3ct-core-test-complex-pipeline-inter-stage -q -t 1 -i ${INPUT_FILE} -p -g)
    set_tests_properties(pipeline1::aff3ct-core-test-complex-pipeline-inter-stage PROPERTIES LABELS complex-pipeline-inter-stage)
    add_test(NAME pipeline2::aff3ct-core-test-complex-pipeline-inter-stage COMMAND aff3ct-core-test-complex-pipeline-inter-stage -i ${INPUT_FILE} -u 1)
    set_tests_properties(pipeline2::aff3ct-core-test-complex-pipeline-inter-stage PROPERTIES LABELS complex-pipeline-inter-stage)

    # pipeline double-chain
    add_test(NAME pipeline0::aff3ct-core-test-pipeline-double-chain COMMAND aff3ct-core-test-pipeline-double-chain)
    set_tests_properties(pipeline0::aff3ct-core-test-pipeline-double-chain PROPERTIES LABELS simple-pipeline-double-chain)
    add_test(NAME pipeline1::aff3ct-core-test-pipeline-double-chain COMMAND aff3ct-core-test-pipeline-double-chain -u 1024)
    set_tests_properties(pipeline1::aff3ct-core-test-pipeline-double-chain PROPERTIES LABELS simple-pipeline-double-chain)

    # complex pipeline fwd
    add_test(NAME sequence0::aff3ct-core-test-complex-pipeline-full-fwd COMMAND aff3ct-core-test-complex-pipeline-full-fwd -e 100 -q -t 1)
    set_tests_properties(sequence0::aff3ct-core-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME sequence1::aff3ct-core-test-complex-pipeline-full-fwd COMMAND aff3ct-core-test-complex-pipeline-full-fwd -e 100 -q)
    set_tests_properties(sequence1::aff3ct-core-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME sequence2::aff3ct-core-test-complex-pipeline-full-fwd COMMAND aff3ct-core-test-complex-pipeline-full-fwd -e 100 -q -b -o graph.dot)
    set_tests_properties(sequence2::aff3ct-core-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME sequence3::aff3ct-core-test-complex-pipeline-full-fwd COMMAND aff3ct-core-test-complex-pipeline-full-fwd -e 100 -q -b -f 5 -t 3)
    set_tests_properties(sequence3::aff3ct-core-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME pipeline0::aff3ct-core-test-complex-pipeline-full-fwd COMMAND aff3ct-core-test-complex-pipeline-full-fwd -e 10 -t 1)
    set_tests_properties(pipeline0::aff3ct-core-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME pipeline1::aff3ct-core-test-complex-pipeline-full-fwd COMMAND aff3ct-core-test-complex-pipeline-full-fwd -e 10)
    set_tests_properties(pipeline1::aff3ct-core-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME pipeline2::aff3ct-core-test-complex-pipeline-full-fwd COMMAND aff3ct-core-test-complex-pipeline-full-fwd -e 1000 -u 128 -o graph.dot)
    set_tests_properties(pipeline2::aff3ct-core-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME pipeline3::aff3ct-core-test-complex-pipeline-full-fwd COMMAND aff3ct-core-test-complex-pipeline-full-fwd -e 1000 -u 49 -f 13)
    set_tests_properties(pipeline3::aff3ct-core-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME pipeline4::aff3ct-core-test-complex-pipeline-full-fwd COMMAND aff3ct-core-test-complex-pipeline-full-fwd -e 1000 -u 413 -w)
    set_tests_properties(pipeline4::aff3ct-core-test-complex-pipeline-full-fwd PROPERTIES LABELS "complex-pipeline-full-fwd;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(

    # complex mix pipeline (fwd + in-out)
    add_test(NAME sequence0::aff3ct-core-test-complex-pipeline-mix-fwd COMMAND aff3ct-core-test-complex-pipeline-mix-fwd -e 100 -q -t 1)
    set_tests_properties(sequence0::aff3ct-core-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME sequence1::aff3ct-core-test-complex-pipeline-mix-fwd COMMAND aff3ct-core-test-complex-pipeline-mix-fwd -e 100 -q)
    set_tests_properties(sequence1::aff3ct-core-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME sequence2::aff3ct-core-test-complex-pipeline-mix-fwd COMMAND aff3ct-core-test-complex-pipeline-mix-fwd -e 100 -q -b -o graph.dot)
    set_tests_properties(sequence2::aff3ct-core-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME sequence3::aff3ct-core-test-complex-pipeline-mix-fwd COMMAND aff3ct-core-test-complex-pipeline-mix-fwd -e 100 -q -f 5 -t 3)
    set_tests_properties(sequence3::aff3ct-core-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME pipeline0::aff3ct-core-test-complex-pipeline-mix-fwd COMMAND aff3ct-core-test-complex-pipeline-mix-fwd -e 10 -t 1)
    set_tests_properties(pipeline0::aff3ct-core-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME pipeline1::aff3ct-core-test-complex-pipeline-mix-fwd COMMAND aff3ct-core-test-complex-pipeline-mix-fwd -e 10)
    set_tests_properties(pipeline1::aff3ct-core-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME pipeline2::aff3ct-core-test-complex-pipeline-mix-fwd COMMAND aff3ct-core-test-complex-pipeline-mix-fwd -e 1000 -u 128 -o graph.dot)
    set_tests_properties(pipeline2::aff3ct-core-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME pipeline3::aff3ct-core-test-complex-pipeline-mix-fwd COMMAND aff3ct-core-test-complex-pipeline-mix-fwd -e 1000 -u 49 -f 13)
    set_tests_properties(pipeline3::aff3ct-core-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME pipeline4::aff3ct-core-test-complex-pipeline-mix-fwd COMMAND aff3ct-core-test-complex-pipeline-mix-fwd -e 1000 -u 413 -w)
    set_tests_properties(pipeline4::aff3ct-core-test-complex-pipeline-mix-fwd PROPERTIES LABELS "complex-pipeline-mix-fwd;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(

    # simple sequence example
    add_test(NAME sequence0::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(3)" -t "(3)" -R "(SIO)" -q)
    set_tests_properties(sequence0::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME sequence1::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(3)" -t "(3)" -R "(SFWD)" -q -b)
    set_tests_properties(sequence1::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME sequence2::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(4)" -t "(3)" -r "((SFWD,SIO,SFWD,SIO))" -q)
    set_tests_properties(sequence2::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME sequence3::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(4,1,2)" -t "(3,1,3)" -r "((SFWD,SIO,SFWD,SIO),(SFWD),(SIO,SIO))" -q -b)
    set_tests_properties(sequence3::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # simple pipeline example
    add_test(NAME pipeline0::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(3)" -t "(3)" -R "(SIO)")
    set_tests_properties(pipeline0::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # simple pipeline FWD example
    add_test(NAME pipeline1::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(3)" -t "(3)" -R "(SFWD)")
    set_tests_properties(pipeline1::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline2::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(3)" -t "(3)" -R "(SFWD)" -p)
    set_tests_properties(pipeline2::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline3::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(3)" -t "(3)" -R "(SFWD)" -w)
    set_tests_properties(pipeline3::aff3ct-core-test-generic-pipeline PROPERTIES LABELS "generic-pipeline;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(
    add_test(NAME pipeline4::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(3)" -t "(3)" -R "(SFWD)" -u 1)
    set_tests_properties(pipeline4::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline5::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(3)" -t "(3)" -R "(SFWD)" -u 1024)
    set_tests_properties(pipeline5::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # simple pipeline hybrid
    add_test(NAME pipeline6::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(4)" -t "(3)" -r "((SFWD,SIO,SFWD,SIO))")
    set_tests_properties(pipeline6::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline7::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(4)" -t "(3)" -r "((SFWD,SIO,SFWD,SIO))" -u 1024)
    set_tests_properties(pipeline7::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    #pipeline multiple stages
    add_test(NAME pipeline8::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(4,1,2)" -t "(3,1,3)" -r "((SFWD,SIO,SFWD,SIO),(SFWD),(SIO,SIO))")
    set_tests_properties(pipeline8::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline9::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(4,1,2)" -t "(3,1,3)" -r "((SFWD,SIO,SFWD,SIO),(SFWD),(SIO,SIO))" -p)
    set_tests_properties(pipeline9::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline10::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(4,1,2)" -t "(3,1,3)" -r "((SFWD,SIO,SFWD,SIO),(SFWD),(SIO,SIO))" -w)
    set_tests_properties(pipeline10::aff3ct-core-test-generic-pipeline PROPERTIES LABELS "generic-pipeline;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(
    add_test(NAME pipeline11::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(4,1,2)" -t "(3,1,3)" -r "((SFWD,SIO,SFWD,SIO),(SFWD),(SIO,SIO))" -u 1)
    set_tests_properties(pipeline11::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline12::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(4,1,2)" -t "(3,1,3)" -r "((SFWD,SIO,SFWD,SIO),(SFWD),(SIO,SIO))" -u 1024)
    set_tests_properties(pipeline12::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # pipeline with socket type per stage
    add_test(NAME pipeline13::aff3ct-core-test-generic-pipeline COMMAND aff3ct-core-test-generic-pipeline -i ${INPUT_FILE} -n "(4,1,2)" -t "(3,1,3)" -R "(SFWD,SIO,SFWD)")
    set_tests_properties(pipeline13::aff3ct-core-test-generic-pipeline PROPERTIES LABELS generic-pipeline)

    add_test(NAME sequence0::aff3ct-core-test-exclusive-paths-pipeline COMMAND aff3ct-core-test-exclusive-paths-pipeline -t 1 -q -i ${INPUT_FILE})
    set_tests_properties(sequence0::aff3ct-core-test-exclusive-paths-pipeline PROPERTIES LABELS exclusive-paths-pipeline)
    add_test(NAME sequence1::aff3ct-core-test-exclusive-paths-pipeline COMMAND aff3ct-core-test-exclusive-paths-pipeline -t 1 -u 13 -q -b -i ${INPUT_FILE})
    set_tests_properties(sequence1::aff3ct-core-test-exclusive-paths-pipeline PROPERTIES LABELS exclusive-paths-pipeline)
    add_test(NAME pipeline0::aff3ct-core-test-exclusive-paths-pipeline COMMAND aff3ct-core-test-exclusive-paths-pipeline -t 4 -u 1 -i ${INPUT_FILE})
    set_tests_properties(pipeline0::aff3ct-core-test-exclusive-paths-pipeline PROPERTIES LABELS exclusive-paths-pipeline)
    add_test(NAME pipeline1::aff3ct-core-test-exclusive-paths-pipeline COMMAND aff3ct-core-test-exclusive-paths-pipeline -t 1 -u 5 -i ${INPUT_FILE})
    set_tests_properties(pipeline1::aff3ct-core-test-exclusive-paths-pipeline PROPERTIES LABELS exclusive-paths-pipeline)
    add_test(NAME pipeline2::aff3ct-core-test-exclusive-paths-pipeline COMMAND aff3ct-core-test-exclusive-paths-pipeline -t 8 -u 15 -i ${INPUT_FILE})
    set_tests_properties(pipeline2::aff3ct-core-test-exclusive-paths-pipeline PROPERTIES LABELS exclusive-paths-pipeline)

    add_test(NAME sequence0::aff3ct-core-test-nest-loops-pipeline COMMAND aff3ct-core-test-nest-loops-pipeline -e 200 -f 13 -q -b -t 12)
    set_tests_properties(sequence0::aff3ct-core-test-nest-loops-pipeline PROPERTIES LABELS nest-loops-pipeline)
    add_test(NAME pipeline0::aff3ct-core-test-nest-loops-pipeline COMMAND aff3ct-core-test-nest-loops-pipeline -e 10 -f 13 -i 10 -j 12 -t 8)
    set_tests_properties(pipeline0::aff3ct-core-test-nest-loops-pipeline PROPERTIES LABELS nest-loops-pipeline)
    add_test(NAME pipeline1::aff3ct-core-test-nest-loops-pipeline COMMAND aff3ct-core-test-nest-loops-pipeline -e 10 -a -i 5 -j 13 -t 1)
    set_tests_properties(pipeline1::aff3ct-core-test-nest-loops-pipeline PROPERTIES LABELS nest-loops-pipeline)
    add_test(NAME pipeline2::aff3ct-core-test-nest-loops-pipeline COMMAND aff3ct-core-test-nest-loops-pipeline -e 200 -i 5 -j 3 -t 2)
    set_tests_properties(pipeline2::aff3ct-core-test-nest-loops-pipeline PROPERTIES LABELS nest-loops-pipeline)
    add_test(NAME pipeline3::aff3ct-core-test-nest-loops-pipeline COMMAND aff3ct-core-test-nest-loops-pipeline -e 200 -f 13)
    set_tests_properties(pipeline3::aff3ct-core-test-nest-loops-pipeline PROPERTIES LABELS nest-loops-pipeline)
endif()
