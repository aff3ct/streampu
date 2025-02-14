# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------- CMAKE PROJECT
# ---------------------------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0054 NEW)

if(${CMAKE_VERSION} VERSION_EQUAL "3.14" OR ${CMAKE_VERSION} VERSION_GREATER "3.14")
    cmake_policy(SET CMP0083 NEW)
endif()

project(streampu CXX)

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------- CMAKE OPTIONS
# ---------------------------------------------------------------------------------------------------------------------

option(SPU_COMPILE_STATIC_LIB  "Compile the static library"                                  ON )
option(SPU_COMPILE_SHARED_LIB  "Compile the shared library"                                  OFF)
option(SPU_LINK_HWLOC          "Link with the hwloc library (used for threads pinning)"      OFF)
option(SPU_COLORS              "Enable the colors in the terminal"                           ON )
option(SPU_TESTS               "Enable the compilation of the tests"                         ON )
option(SPU_STACKTRACE          "Print the stack trace when an exception is raised"           ON )
option(SPU_STACKTRACE_SEGFAULT "Try to print the stack trace when a segfault occurs"         OFF)
option(SPU_SHOW_DEPRECATED     "Print message each time a deprecated func. is called"        OFF)
option(SPU_FAST                "Remove checks to speedup the code"                           OFF)
option(SPU_OVERRIDE_VERSION    "Compile without .git directory, provided a version and hash" OFF)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git" AND NOT SPU_OVERRIDE_VERSION)
    message(FATAL_ERROR "The '.git' folder can't be found, StreamPU can't be compiled if it is not cloned "
                        "from a Git repository. Please do not download archives from GitHub and make a Git "
                        "clone instead (git clone https://github.com/aff3ct/streampu.git).")
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
# ------------------------------------------------------------------------------------------------ GET VERSION FROM GIT
# ---------------------------------------------------------------------------------------------------------------------

if(NOT SPU_OVERRIDE_VERSION)
    include(GetGitRevisionDescription)
    get_git_head_revision(GIT_REFSPEC GIT_SHA1)
    git_describe(GIT_VERSION "--tags" "--abbrev=7")
else()
    set(GIT_VERSION ${SPU_OVERRIDE_VERSION})
endif()

string(REGEX REPLACE "^v"             ""    SPU_VERSION_FULL  ${GIT_VERSION})
string(REGEX REPLACE "-.*"            ""    SPU_VERSION       ${SPU_VERSION_FULL})
string(REGEX REPLACE "^.*-"           ""    SPU_HASH          ${SPU_VERSION_FULL})
string(      REPLACE "-"              ""    SPU_BUILD         ${SPU_VERSION_FULL})
string(      REPLACE "${SPU_VERSION}" ""    SPU_BUILD         ${SPU_BUILD})
string(REGEX REPLACE "(.*)\\..*\\..*" "\\1" SPU_VERSION_MAJOR ${SPU_VERSION})
string(REGEX REPLACE ".*\\.(.*)\\..*" "\\1" SPU_VERSION_MINOR ${SPU_VERSION})
string(REGEX REPLACE ".*\\..*\\.(.*)" "\\1" SPU_VERSION_PATCH ${SPU_VERSION})

if(NOT "${SPU_BUILD}" STREQUAL "")
    string(REPLACE "${SPU_HASH}" "" SPU_BUILD ${SPU_BUILD})
else()
    set(SPU_HASH "")
endif()

message(STATUS "StreamPU - Version: ${SPU_VERSION}")
if(NOT "${SPU_BUILD}" STREQUAL "")
    message(STATUS "StreamPU - Build: ${SPU_BUILD}")
endif()
if(NOT "${SPU_HASH}" STREQUAL "")
    message(STATUS "StreamPU - Hash: ${SPU_HASH}")
endif()

if(("${SPU_BUILD}" STREQUAL "") OR ("${SPU_HASH}" STREQUAL ""))
    set(SPU_VERSION_EXTRA "")
else()
    set(SPU_VERSION_EXTRA "-${SPU_BUILD}-${SPU_HASH}")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------ GENERATE FILES
# ---------------------------------------------------------------------------------------------------------------------

# Auto generate cmake config version file to link with StreamPU lib (only if an StreamPU library has been compiled)
if (SPU_COMPILE_STATIC_LIB OR SPU_COMPILE_SHARED_LIB)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/streampu-config-version.cmake.in"
                   "lib/cmake/streampu/streampu-config-version.cmake" @ONLY)
endif()

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

add_library(spu-obj OBJECT ${source_files})
set_target_properties(spu-obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
list(APPEND spu_targets_list spu-obj)

# Library
if(SPU_COMPILE_SHARED_LIB)
    add_library(spu-shared-lib SHARED $<TARGET_OBJECTS:spu-obj>)
    set_target_properties(spu-shared-lib PROPERTIES OUTPUT_NAME streampu POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-shared-lib)
    message(STATUS "StreamPU - Compile: shared library")
endif(SPU_COMPILE_SHARED_LIB)
if(SPU_COMPILE_STATIC_LIB)
    add_library(spu-static-lib STATIC $<TARGET_OBJECTS:spu-obj>)
    set_target_properties(spu-static-lib PROPERTIES OUTPUT_NAME streampu POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-static-lib)
    message(STATUS "StreamPU - Compile: static library")
endif(SPU_COMPILE_STATIC_LIB)

if(SPU_STACKTRACE_SEGFAULT AND CPPTRACE_UNWIND_WITH_LIBUNWIND)
    add_executable(spu-signal-tracer ${CMAKE_CURRENT_SOURCE_DIR}/signal_tracer/main.cpp)
    set_target_properties(spu-signal-tracer PROPERTIES OUTPUT_NAME streampu-signal-tracer POSITION_INDEPENDENT_CODE ON)
endif()

# Tests
if(SPU_TESTS)
    add_executable(spu-test-simple-chain $<TARGET_OBJECTS:spu-obj> ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/simple_chain.cpp)
    set_target_properties(spu-test-simple-chain PROPERTIES OUTPUT_NAME test-simple-chain POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_tests_list spu-test-simple-chain)
    list(APPEND spu_targets_list spu-test-simple-chain)

    add_executable(spu-test-simple-chain-fwd $<TARGET_OBJECTS:spu-obj> ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/simple_chain_fwd.cpp)
    set_target_properties(spu-test-simple-chain-fwd PROPERTIES OUTPUT_NAME test-simple-chain-fwd POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_tests_list spu-test-simple-chain-fwd)
    list(APPEND spu_targets_list spu-test-simple-chain-fwd)

    add_executable(spu-test-simple-chain-hybrid $<TARGET_OBJECTS:spu-obj> ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/simple_chain_hybrid.cpp)
    set_target_properties(spu-test-simple-chain-hybrid PROPERTIES OUTPUT_NAME test-simple-chain-hybrid POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_tests_list spu-test-simple-chain-hybrid)
    list(APPEND spu_targets_list spu-test-simple-chain-hybrid)

    add_executable(spu-test-for-loop $<TARGET_OBJECTS:spu-obj> ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/for_loop.cpp)
    set_target_properties(spu-test-for-loop PROPERTIES OUTPUT_NAME test-for-loop POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_tests_list spu-test-for-loop)
    list(APPEND spu_targets_list spu-test-for-loop)

    add_executable(spu-test-do-while-loop $<TARGET_OBJECTS:spu-obj> ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/do_while_loop.cpp)
    set_target_properties(spu-test-do-while-loop PROPERTIES OUTPUT_NAME test-do-while-loop POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_tests_list spu-test-do-while-loop)
    list(APPEND spu_targets_list spu-test-do-while-loop)

    add_executable(spu-test-exclusive-paths $<TARGET_OBJECTS:spu-obj> ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/exclusive_paths.cpp)
    set_target_properties(spu-test-exclusive-paths PROPERTIES OUTPUT_NAME test-exclusive-paths POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_tests_list spu-test-exclusive-paths)
    list(APPEND spu_targets_list spu-test-exclusive-paths)

    add_executable(spu-test-nested-loops $<TARGET_OBJECTS:spu-obj> ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/nested_loops.cpp)
    set_target_properties(spu-test-nested-loops PROPERTIES OUTPUT_NAME test-nested-loops POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_tests_list spu-test-nested-loops)
    list(APPEND spu_targets_list spu-test-nested-loops)

    add_executable(spu-test-nested-do-while-loops $<TARGET_OBJECTS:spu-obj> ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/nested_do_while_loops.cpp)
    set_target_properties(spu-test-nested-do-while-loops PROPERTIES OUTPUT_NAME test-nested-do-while-loops POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_tests_list spu-test-nested-do-while-loops)
    list(APPEND spu_targets_list spu-test-nested-do-while-loops)

    add_executable(spu-test-simple-pipeline $<TARGET_OBJECTS:spu-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap/simple_pipeline.cpp)
    set_target_properties(spu-test-simple-pipeline PROPERTIES OUTPUT_NAME test-simple-pipeline POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-test-simple-pipeline)

    add_executable(spu-test-pipeline-probe $<TARGET_OBJECTS:spu-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced/pipeline_probe.cpp)
    set_target_properties(spu-test-pipeline-probe PROPERTIES OUTPUT_NAME test-pipeline-probe POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-test-pipeline-probe)

    add_executable(spu-test-complex-pipeline-inter-stage $<TARGET_OBJECTS:spu-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced/complex_pipeline_inter_stage.cpp)
    set_target_properties(spu-test-complex-pipeline-inter-stage PROPERTIES OUTPUT_NAME test-complex-pipeline-inter-stage POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-test-complex-pipeline-inter-stage)

    add_executable(spu-test-pipeline-double-chain $<TARGET_OBJECTS:spu-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced/pipeline_double_chain.cpp)
    set_target_properties(spu-test-pipeline-double-chain PROPERTIES OUTPUT_NAME test-pipeline-double-chain POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-test-pipeline-double-chain)

    add_executable(spu-test-complex-pipeline-full-fwd $<TARGET_OBJECTS:spu-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced/complex_pipeline_full_fwd.cpp)
    set_target_properties(spu-test-complex-pipeline-full-fwd PROPERTIES OUTPUT_NAME test-complex-pipeline-full-fwd POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-test-complex-pipeline-full-fwd)

    add_executable(spu-test-complex-pipeline-mix-fwd $<TARGET_OBJECTS:spu-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced/complex_pipeline_mix_fwd.cpp)
    set_target_properties(spu-test-complex-pipeline-mix-fwd PROPERTIES OUTPUT_NAME test-complex-pipeline-mix-fwd POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-test-complex-pipeline-mix-fwd)

    add_executable(spu-test-generic-pipeline $<TARGET_OBJECTS:spu-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced/generic_pipeline.cpp)
    set_target_properties(spu-test-generic-pipeline PROPERTIES OUTPUT_NAME test-generic-pipeline POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-test-generic-pipeline)

    add_executable(spu-test-exclusive-paths-pipeline $<TARGET_OBJECTS:spu-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/ctrl_flow/exclusive_paths_pipeline.cpp)
    set_target_properties(spu-test-exclusive-paths-pipeline PROPERTIES OUTPUT_NAME test-exclusive-paths-pipeline POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-test-exclusive-paths-pipeline)

    add_executable(spu-test-nest-loops-pipeline $<TARGET_OBJECTS:spu-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/ctrl_flow/nested_loops_pipeline.cpp)
    set_target_properties(spu-test-nest-loops-pipeline PROPERTIES OUTPUT_NAME test-nested-loops-pipeline POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-test-nest-loops-pipeline)

    add_executable(spu-test-pipeline-builder-simple $<TARGET_OBJECTS:spu-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/builder/pipeline_builder_simple.cpp)
    set_target_properties(spu-test-pipeline-builder-simple PROPERTIES OUTPUT_NAME test-pipeline-builder-simple POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-test-pipeline-builder-simple)

    add_executable(spu-test-pipeline-builder-conditional $<TARGET_OBJECTS:spu-obj>
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/builder/pipeline_builder_conditional.cpp)
    set_target_properties(spu-test-pipeline-builder-conditional PROPERTIES OUTPUT_NAME test-pipeline-builder-conditional POSITION_INDEPENDENT_CODE ON)
    list(APPEND spu_targets_list spu-test-pipeline-builder-conditional)

    if (SPU_LINK_HWLOC)
        # pin test addition
        add_executable(spu-test-thread-pinning $<TARGET_OBJECTS:spu-obj> ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced/thread_pinning.cpp)
        set_target_properties(spu-test-thread-pinning PROPERTIES OUTPUT_NAME test-thread-pinning POSITION_INDEPENDENT_CODE ON)
        list(APPEND spu_targets_list spu-test-thread-pinning)
    endif (SPU_LINK_HWLOC)
endif()

# ---------------------------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------------- SUB-PROJECTS
# ---------------------------------------------------------------------------------------------------------------------

# cpptrace
if (SPU_STACKTRACE OR SPU_STACKTRACE_SEGFAULT)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/cpptrace/include/cpptrace/cpptrace.hpp")
        option(CPPTRACE_GET_SYMBOLS_WITH_LIBDWARF "" OFF)
        option(CPPTRACE_GET_SYMBOLS_WITH_ADDR2LINE "" ON)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/cpptrace/)
    else()
        message(FATAL_ERROR "'cpptrace' can't be found, try to init the submodule with the following cmd:\n"
                            "$ git submodule update --init -- ../lib/cpptrace/")
    endif()
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------ COMPILER DEFINITIONS
# ---------------------------------------------------------------------------------------------------------------------

macro(spu_target_compile_definitions targets privacy def)
    foreach(_target IN ITEMS ${targets})
        target_compile_definitions(${_target} ${privacy} $<BUILD_INTERFACE:${def}> $<INSTALL_INTERFACE:${def}>)
    endforeach()
endmacro()

# by system
if(WIN32) # for Windows operating system in general
    set(WINDOWS_VISTA 0x0600)
    spu_target_compile_definitions("${spu_targets_list}" PUBLIC _WIN32_WINNT=${WINDOWS_VISTA})
    spu_target_compile_definitions("${spu_targets_list}" PUBLIC NOMINMAX)
    message(STATUS "StreamPU - System: Windows")
elseif(APPLE) # for macOS
    message(STATUS "StreamPU - System: macOS")
elseif(UNIX AND NOT APPLE) # for Linux, BSD, Solaris, Minix
    message(STATUS "StreamPU - System: Unix/Linux")
endif()

# common
if(SPU_COLORS)
    spu_target_compile_definitions("${spu_targets_list}" PUBLIC SPU_COLORS)
    message(STATUS "StreamPU - Terminal colors: on")
else()
    message(STATUS "StramPU - Terminal colors: off")
endif()

if(SPU_STACKTRACE)
    spu_target_compile_definitions("${spu_targets_list}" PUBLIC SPU_STACKTRACE)
    message(STATUS "StreamPU - Stacktrace: on")
else()
    message(STATUS "StreamPU - Stacktrace: off")
endif()

if (SPU_STACKTRACE_SEGFAULT)
    spu_target_compile_definitions("${spu_targets_list}" PUBLIC SPU_STACKTRACE_SEGFAULT)
    if(CPPTRACE_UNWIND_WITH_LIBUNWIND)
        spu_target_compile_definitions("${spu_targets_list}" PUBLIC SPU_STACKTRACE_SEGFAULT_LIBUNWIND)
        if(SPU_COLORS)
            target_compile_definitions(streampu-signal-tracer PRIVATE SPU_COLORS)
        endif()
        message(STATUS "StreamPU - Stacktrace segfault: on [with libunwind -> experimental]")
    else()
        message(STATUS "StreamPU - Stacktrace segfault: on [unsafe method]")
    endif(CPPTRACE_UNWIND_WITH_LIBUNWIND)
else()
    message(STATUS "StreamPU - Stacktrace segfault: off")
endif()

if(SPU_SHOW_DEPRECATED)
    spu_target_compile_definitions("${spu_targets_list}" PUBLIC SPU_SHOW_DEPRECATED)
    message(STATUS "StreamPU - Show deprecated: on")
else()
    message(STATUS "StreamPU - Show deprecated: off")
endif()

if(SPU_FAST)
    spu_target_compile_definitions("${spu_targets_list}" PUBLIC SPU_FAST)
    message(STATUS "StreamPU - Fast: on")
else()
    message(STATUS "StreamPU - Fast: off")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------- HEADER ONLY LIBRARIES
# ---------------------------------------------------------------------------------------------------------------------

macro(spu_target_include_directories targets privacy dir_build dir_install)
    foreach(_target IN ITEMS ${targets})
        target_include_directories(${_target}
                                   ${privacy}
                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir_build}/>
                                   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${dir_install}>)
    endforeach()
endmacro()

macro(spu_target_include_directories2 targets privacy dir)
    foreach(_target IN ITEMS ${targets})
        target_include_directories(${_target} ${privacy} $<BUILD_INTERFACE:${dir}> $<INSTALL_INTERFACE:${dir}>)
    endforeach()
endmacro()

# StreamPU headers
spu_target_include_directories("${spu_targets_list}" PRIVATE "src"     "streampu")
spu_target_include_directories("${spu_targets_list}" PUBLIC  "include" "streampu")

# rang
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/rang/include/rang.hpp")
    spu_target_include_directories("${spu_targets_list}" PUBLIC "lib/rang/include" "rang")
    message(STATUS "StreamPU - Header found: rang")
else()
    message(FATAL_ERROR "StreamPU - rang can't be found, try to init the submodule with the following cmd:\n"
                        "$ git submodule update --init -- ../lib/rang/")
endif()

# nlohmann json
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/json/include/nlohmann/json.hpp")
    spu_target_include_directories("${spu_targets_list}" PUBLIC "lib/json/include/" "json")
    message(STATUS "StreamPU - Header found: nlohmann_json")
else()
    message(FATAL_ERROR "StreamPU - nlohmann_json can't be found, try to init the submodule with the following cmd:\n"
                        "$ git submodule update --init -- ../lib/json/")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------- COMPILED LIBRARIES
# ---------------------------------------------------------------------------------------------------------------------

macro(spu_target_link_libraries targets privacy lib)
    foreach(_target IN ITEMS ${targets})
        target_link_libraries(${_target} ${privacy} ${lib})
    endforeach()
endmacro()

# hwloc
if(SPU_LINK_HWLOC)
    spu_target_compile_definitions("${spu_targets_list}" PUBLIC "SPU_HWLOC")

    find_package(Hwloc REQUIRED QUIET)
    if(Hwloc_FOUND)
        message(STATUS "StramPU - Library found: hwloc")
        spu_target_include_directories2("${spu_targets_list}" PUBLIC "${Hwloc_INCLUDE_DIRS}")
        spu_target_link_libraries("${spu_targets_list}" PUBLIC "${Hwloc_LIBRARIES}")
    endif(Hwloc_FOUND)
endif(SPU_LINK_HWLOC)

# cpptrace
if(SPU_STACKTRACE OR SPU_STACKTRACE_SEGFAULT)
    spu_target_link_libraries("${spu_targets_list}" PUBLIC "cpptrace::cpptrace")
    if(SPU_STACKTRACE_SEGFAULT AND CPPTRACE_UNWIND_WITH_LIBUNWIND)
        target_link_libraries(streampu-signal-tracer PRIVATE "cpptrace::cpptrace")
    endif()
endif()

# Threads
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
spu_target_link_libraries("${spu_targets_list}" PUBLIC Threads::Threads)

# ---------------------------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------------------- EXPORT
# ---------------------------------------------------------------------------------------------------------------------

if(SPU_STACKTRACE OR SPU_STACKTRACE_SEGFAULT)
    set(CPPTRACE_TARGETS cpptrace-lib)
    if (CPPTRACE_GET_SYMBOLS_WITH_LIBDWARF)
        set(CPPTRACE_TARGETS ${CPPTRACE_TARGETS} dwarf)
    endif()
else()
    set(CPPTRACE_TARGETS)
endif()

if (SPU_COMPILE_SHARED_LIB AND NOT SPU_COMPILE_STATIC_LIB)
    export(TARGETS spu-shared-lib ${CPPTRACE_TARGETS}
           NAMESPACE spu::
           FILE "lib/cmake/streampu/streampu-config.cmake")
endif()

if (SPU_COMPILE_STATIC_LIB AND NOT SPU_COMPILE_SHARED_LIB)
    export(TARGETS spu-static-lib ${CPPTRACE_TARGETS}
           NAMESPACE spu::
           FILE "lib/cmake/streampu/streampu-config.cmake")
endif()

if(SPU_COMPILE_SHARED_LIB AND SPU_COMPILE_STATIC_LIB)
    export(TARGETS spu-shared-lib spu-static-lib ${CPPTRACE_TARGETS}
           NAMESPACE spu::
           FILE "lib/cmake/streampu/streampu-config.cmake")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------- INSTALL
# ---------------------------------------------------------------------------------------------------------------------

if(SPU_COMPILE_SHARED_LIB)
    if(WIN32)
        install(TARGETS spu-shared-lib
                EXPORT streampu-config
                RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
                COMPONENT library)
    else()
        install(TARGETS spu-shared-lib
                EXPORT streampu-config
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
                COMPONENT library)
    endif()
endif()

if(SPU_COMPILE_STATIC_LIB)
    install(TARGETS spu-static-lib
            EXPORT streampu-config
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
            COMPONENT library)
endif()

if (SPU_COMPILE_SHARED_LIB OR SPU_COMPILE_STATIC_LIB)
    install(EXPORT
            streampu-config
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/streampu/"
            NAMESPACE spu::
            COMPONENT library)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/streampu/streampu-config-version.cmake"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/streampu/"
            COMPONENT library)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/streampu/
            COMPONENT headers
            FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/streampu/
            COMPONENT headers
            FILES_MATCHING PATTERN "*.hpp")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/streampu/
            COMPONENT headers
            FILES_MATCHING PATTERN "*.hxx")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/rang/include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rang
            COMPONENT headers
            FILES_MATCHING PATTERN "*.hpp")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/json/single_include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/json
            COMPONENT headers
            FILES_MATCHING PATTERN "*.hpp")
endif()

# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------------------- TEST
# ---------------------------------------------------------------------------------------------------------------------

if(SPU_TESTS)
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full --show-leak-kinds=all")
    include(CTest)
    enable_testing()

    macro(spu_run_1t_tests targets)
        foreach(_target IN ITEMS ${targets})
            add_test(NAME run-1t::${_target} COMMAND ${_target} -d 2048 -s 5 -e 10 -t 1)
            set_tests_properties(run-1t::${_target} PROPERTIES LABELS run-1t)
        endforeach()
    endmacro()

    spu_run_1t_tests("${spu_tests_list}")

    macro(spu_run_tests targets)
        foreach(_target IN ITEMS ${targets})
            add_test(NAME run::${_target} COMMAND ${_target} -d 2048 -s 5 -e 100)
            set_tests_properties(run::${_target} PROPERTIES LABELS run)
        endforeach()
    endmacro()

    spu_run_tests("${spu_tests_list}")

    macro(spu_inter_frames_tests targets)
        foreach(_target IN ITEMS ${targets})
            add_test(NAME inter-frames::${_target} COMMAND ${_target} -d 2048 -s 5 -e 10 -f 13)
            set_tests_properties(inter-frames::${_target} PROPERTIES LABELS inter-frames)
        endforeach()
    endmacro()

    spu_inter_frames_tests("${spu_tests_list}")

    macro(spu_step_by_step_tests targets)
        foreach(_target IN ITEMS ${targets})
            add_test(NAME step-by-step::${_target} COMMAND ${_target} -d 2048 -s 5 -e 11 -f 13 -b)
            set_tests_properties(step-by-step::${_target} PROPERTIES LABELS step-by-step)
        endforeach()
    endmacro()

    spu_step_by_step_tests("${spu_tests_list}")

    macro(spu_copy_mode_tests targets)
        foreach(_target IN ITEMS ${targets})
            add_test(NAME copy-mode::${_target} COMMAND ${_target} -d 2048 -s 5 -e 15 -f 13 -c)
            set_tests_properties(copy-mode::${_target} PROPERTIES LABELS copy-mode)
        endforeach()
    endmacro()

    spu_copy_mode_tests("${spu_tests_list}")

    macro(spu_graph_dot_tests targets)
        foreach(_target IN ITEMS ${targets})
            add_test(NAME graph-dot::${_target} COMMAND ${_target} -d 2048 -s 5 -e 15 -f 13 -o graph.dot)
            set_tests_properties(graph-dot::${_target} PROPERTIES LABELS graph-dot)
        endforeach()
    endmacro()

    spu_graph_dot_tests("${spu_tests_list}")

    set(INPUT_FILE "../mkdocs.yml")

    add_test(NAME set0::spu-test-simple-chain COMMAND spu-test-simple-chain -d 2048 -s 5 -e 15 -f 13 -u)
    set_tests_properties(set0::spu-test-simple-chain PROPERTIES LABELS set)
    add_test(NAME set1::spu-test-simple-chain COMMAND spu-test-simple-chain -d 2048 -s 5 -v)
    set_tests_properties(set1::spu-test-simple-chain PROPERTIES LABELS set)

    add_test(NAME path0::spu-test-exclusive-paths COMMAND spu-test-exclusive-paths -d 2048 -s 5 -e 10 -f 13 -a 0)
    set_tests_properties(path0::spu-test-exclusive-paths PROPERTIES LABELS exclusive-paths)
    add_test(NAME path1::spu-test-exclusive-paths COMMAND spu-test-exclusive-paths -d 2048 -s 5 -e 10 -f 13 -a 1)
    set_tests_properties(path1::spu-test-exclusive-paths PROPERTIES LABELS exclusive-paths)
    add_test(NAME path2::spu-test-exclusive-paths COMMAND spu-test-exclusive-paths -d 2048 -s 5 -e 10 -f 13 -a 2)
    set_tests_properties(path2::spu-test-exclusive-paths PROPERTIES LABELS exclusive-paths)
    add_test(NAME cyclic::spu-test-exclusive-paths COMMAND spu-test-exclusive-paths -d 2048 -s 5 -e 10 -f 13 -y)
    set_tests_properties(cyclic::spu-test-exclusive-paths PROPERTIES LABELS exclusive-paths)

    add_test(NAME sequence0::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -q -t 1 -i ${INPUT_FILE})
    set_tests_properties(sequence0::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME sequence1::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -q -t 1 -i ${INPUT_FILE} -p -g)
    set_tests_properties(sequence1::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME sequence2::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -q -t 1 -i ${INPUT_FILE} -b)
    set_tests_properties(sequence2::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME sequence3::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -q -t 1 -i ${INPUT_FILE} -o graph.dot)
    set_tests_properties(sequence3::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME sequence4::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -q -t 1 -i ${INPUT_FILE} -f 2)
    set_tests_properties(sequence4::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME sequence5::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -q -t 1 -i ${INPUT_FILE} -f 13)
    set_tests_properties(sequence5::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline0::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -i ${INPUT_FILE})
    set_tests_properties(pipeline0::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline1::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -i ${INPUT_FILE} -p)
    set_tests_properties(pipeline1::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline2::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -i ${INPUT_FILE} -w)
    set_tests_properties(pipeline2::spu-test-simple-pipeline PROPERTIES LABELS "simple-pipeline;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(
    add_test(NAME pipeline3::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -i ${INPUT_FILE} -u 1)
    set_tests_properties(pipeline3::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline4::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -i ${INPUT_FILE} -u 1024)
    set_tests_properties(pipeline4::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline5::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -i ${INPUT_FILE} -u 128 -o graph.dot)
    set_tests_properties(pipeline5::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline6::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -i ${INPUT_FILE} -u 8 -f 2)
    set_tests_properties(pipeline6::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline7::spu-test-simple-pipeline COMMAND spu-test-simple-pipeline -i ${INPUT_FILE} -u 17 -f 5)
    set_tests_properties(pipeline7::spu-test-simple-pipeline PROPERTIES LABELS simple-pipeline)

    # probes
    add_test(NAME sequence0::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -q -t 1 -i ${INPUT_FILE})
    set_tests_properties(sequence0::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)
    add_test(NAME sequence1::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -q -t 1 -i ${INPUT_FILE} -p -g)
    set_tests_properties(sequence1::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)
    add_test(NAME sequence2::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -q -t 1 -i ${INPUT_FILE} -b)
    set_tests_properties(sequence2::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)
    add_test(NAME sequence3::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -q -t 1 -i ${INPUT_FILE} -o graph.dot)
    set_tests_properties(sequence3::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)
    add_test(NAME sequence4::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -q -t 1 -i ${INPUT_FILE} -f 2)
    set_tests_properties(sequence4::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)
    add_test(NAME sequence5::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -q -t 1 -i ${INPUT_FILE} -f 13)
    set_tests_properties(sequence5::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)
    add_test(NAME pipeline0::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -i ${INPUT_FILE})
    set_tests_properties(pipeline0::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)
    add_test(NAME pipeline1::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -i ${INPUT_FILE} -p)
    set_tests_properties(pipeline1::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)
    add_test(NAME pipeline2::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -i ${INPUT_FILE} -w)
    set_tests_properties(pipeline2::spu-test-pipeline-probe PROPERTIES LABELS "pipeline-probe;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(
    add_test(NAME pipeline3::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -i ${INPUT_FILE} -u 1)
    set_tests_properties(pipeline3::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)
    add_test(NAME pipeline4::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -i ${INPUT_FILE} -u 1024)
    set_tests_properties(pipeline4::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)
    add_test(NAME pipeline5::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -i ${INPUT_FILE} -u 128 -o graph.dot)
    set_tests_properties(pipeline5::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)
    add_test(NAME pipeline6::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -i ${INPUT_FILE} -u 8 -f 2)
    set_tests_properties(pipeline6::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)
    add_test(NAME pipeline7::spu-test-pipeline-probe COMMAND spu-test-pipeline-probe -i ${INPUT_FILE} -u 17 -f 5)
    set_tests_properties(pipeline7::spu-test-pipeline-probe PROPERTIES LABELS pipeline-probe)

    # complex pipeline interstage
    add_test(NAME pipeline0::spu-test-complex-pipeline-inter-stage COMMAND spu-test-complex-pipeline-inter-stage -i ${INPUT_FILE})
    set_tests_properties(pipeline0::spu-test-complex-pipeline-inter-stage PROPERTIES LABELS complex-pipeline-inter-stage)
    add_test(NAME pipeline1::spu-test-complex-pipeline-inter-stage COMMAND spu-test-complex-pipeline-inter-stage -q -t 1 -i ${INPUT_FILE} -p -g)
    set_tests_properties(pipeline1::spu-test-complex-pipeline-inter-stage PROPERTIES LABELS complex-pipeline-inter-stage)
    add_test(NAME pipeline2::spu-test-complex-pipeline-inter-stage COMMAND spu-test-complex-pipeline-inter-stage -i ${INPUT_FILE} -u 1)
    set_tests_properties(pipeline2::spu-test-complex-pipeline-inter-stage PROPERTIES LABELS complex-pipeline-inter-stage)

    # pipeline double-chain
    add_test(NAME sequence0::spu-test-pipeline-double-chain COMMAND spu-test-pipeline-double-chain -e 30 -q -t 1)
    set_tests_properties(sequence0::spu-test-pipeline-double-chain PROPERTIES LABELS pipeline-double-chain)
    add_test(NAME sequence1::spu-test-pipeline-double-chain COMMAND spu-test-pipeline-double-chain -e 19 -q -t 3)
    set_tests_properties(sequence1::spu-test-pipeline-double-chain PROPERTIES LABELS pipeline-double-chain)
    add_test(NAME sequence2::spu-test-pipeline-double-chain COMMAND spu-test-pipeline-double-chain -e 19 -q -t 3 -f 7)
    set_tests_properties(sequence2::spu-test-pipeline-double-chain PROPERTIES LABELS pipeline-double-chain)
    add_test(NAME sequence3::spu-test-pipeline-double-chain COMMAND spu-test-pipeline-double-chain -e 19 -q -t 3 -f 7 -b)
    set_tests_properties(sequence3::spu-test-pipeline-double-chain PROPERTIES LABELS pipeline-double-chain)
    add_test(NAME pipeline0::spu-test-pipeline-double-chain COMMAND spu-test-pipeline-double-chain -e 30)
    set_tests_properties(pipeline0::spu-test-pipeline-double-chain PROPERTIES LABELS pipeline-double-chain)
    add_test(NAME pipeline1::spu-test-pipeline-double-chain COMMAND spu-test-pipeline-double-chain -e 115 -u 1024)
    set_tests_properties(pipeline1::spu-test-pipeline-double-chain PROPERTIES LABELS pipeline-double-chain)
    add_test(NAME pipeline2::spu-test-pipeline-double-chain COMMAND spu-test-pipeline-double-chain -e 10 -u 15 -f 3)
    set_tests_properties(pipeline2::spu-test-pipeline-double-chain PROPERTIES LABELS pipeline-double-chain)

    # complex pipeline fwd
    add_test(NAME sequence0::spu-test-complex-pipeline-full-fwd COMMAND spu-test-complex-pipeline-full-fwd -e 100 -q -t 1)
    set_tests_properties(sequence0::spu-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME sequence1::spu-test-complex-pipeline-full-fwd COMMAND spu-test-complex-pipeline-full-fwd -e 100 -q)
    set_tests_properties(sequence1::spu-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME sequence2::spu-test-complex-pipeline-full-fwd COMMAND spu-test-complex-pipeline-full-fwd -e 100 -q -b -o graph.dot)
    set_tests_properties(sequence2::spu-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME sequence3::spu-test-complex-pipeline-full-fwd COMMAND spu-test-complex-pipeline-full-fwd -e 100 -q -b -f 5 -t 3)
    set_tests_properties(sequence3::spu-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME pipeline0::spu-test-complex-pipeline-full-fwd COMMAND spu-test-complex-pipeline-full-fwd -e 10 -t 1)
    set_tests_properties(pipeline0::spu-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME pipeline1::spu-test-complex-pipeline-full-fwd COMMAND spu-test-complex-pipeline-full-fwd -e 10)
    set_tests_properties(pipeline1::spu-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME pipeline2::spu-test-complex-pipeline-full-fwd COMMAND spu-test-complex-pipeline-full-fwd -e 1000 -u 128 -o graph.dot)
    set_tests_properties(pipeline2::spu-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME pipeline3::spu-test-complex-pipeline-full-fwd COMMAND spu-test-complex-pipeline-full-fwd -e 1000 -u 49 -f 13)
    set_tests_properties(pipeline3::spu-test-complex-pipeline-full-fwd PROPERTIES LABELS complex-pipeline-full-fwd)
    add_test(NAME pipeline4::spu-test-complex-pipeline-full-fwd COMMAND spu-test-complex-pipeline-full-fwd -e 1000 -u 413 -w)
    set_tests_properties(pipeline4::spu-test-complex-pipeline-full-fwd PROPERTIES LABELS "complex-pipeline-full-fwd;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(

    # complex mix pipeline (fwd + in-out)
    add_test(NAME sequence0::spu-test-complex-pipeline-mix-fwd COMMAND spu-test-complex-pipeline-mix-fwd -e 100 -q -t 1)
    set_tests_properties(sequence0::spu-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME sequence1::spu-test-complex-pipeline-mix-fwd COMMAND spu-test-complex-pipeline-mix-fwd -e 100 -q)
    set_tests_properties(sequence1::spu-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME sequence2::spu-test-complex-pipeline-mix-fwd COMMAND spu-test-complex-pipeline-mix-fwd -e 100 -q -b -o graph.dot)
    set_tests_properties(sequence2::spu-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME sequence3::spu-test-complex-pipeline-mix-fwd COMMAND spu-test-complex-pipeline-mix-fwd -e 100 -q -f 5 -t 3)
    set_tests_properties(sequence3::spu-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME pipeline0::spu-test-complex-pipeline-mix-fwd COMMAND spu-test-complex-pipeline-mix-fwd -e 10 -t 1)
    set_tests_properties(pipeline0::spu-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME pipeline1::spu-test-complex-pipeline-mix-fwd COMMAND spu-test-complex-pipeline-mix-fwd -e 10)
    set_tests_properties(pipeline1::spu-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME pipeline2::spu-test-complex-pipeline-mix-fwd COMMAND spu-test-complex-pipeline-mix-fwd -e 1000 -u 128 -o graph.dot)
    set_tests_properties(pipeline2::spu-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME pipeline3::spu-test-complex-pipeline-mix-fwd COMMAND spu-test-complex-pipeline-mix-fwd -e 1000 -u 49 -f 13)
    set_tests_properties(pipeline3::spu-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)
    add_test(NAME pipeline4::spu-test-complex-pipeline-mix-fwd COMMAND spu-test-complex-pipeline-mix-fwd -e 1000 -u 413 -w)
    set_tests_properties(pipeline4::spu-test-complex-pipeline-mix-fwd PROPERTIES LABELS "complex-pipeline-mix-fwd;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(
    add_test(NAME pipeline5::spu-test-complex-pipeline-mix-fwd COMMAND spu-test-complex-pipeline-mix-fwd -t 12 -o dot2.dot -S "OTAC" -v)
    set_tests_properties(pipeline5::spu-test-complex-pipeline-mix-fwd PROPERTIES LABELS complex-pipeline-mix-fwd)

    # simple sequence example
    add_test(NAME sequence0::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -n "1,3,1" -t "1" -R "(read,relay,write)" -q)
    set_tests_properties(sequence0::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME sequence1::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -n "1,3,1" -t "1" -R "(read,relayf,write)" -q -b)
    set_tests_properties(sequence1::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME sequence2::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -t "1" -r "((read),(relayf,relay,relayf,relay),(write))" -q)
    set_tests_properties(sequence2::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME sequence3::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -t "1,3,1,3,1" -r "((read),(relayf,relay,relayf,relay),(relayf),(relay,relay),(write))" -q -b)
    set_tests_properties(sequence3::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # simple pipeline example
    add_test(NAME pipeline0::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -n "1,3,1" -t "1,3,1" -R "(read,relay,write)")
    set_tests_properties(pipeline0::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # simple pipeline FWD example
    add_test(NAME pipeline1::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -n "1,3,1" -t "1,3,1" -R "(read,relayf,write)")
    set_tests_properties(pipeline1::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline2::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -n "1,3,1" -t "1,3,1" -R "(read,relayf,write)" -p)
    set_tests_properties(pipeline2::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline3::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -n "1,3,1" -t "1,3,1" -R "(read,relayf,write)" -w)
    set_tests_properties(pipeline3::spu-test-generic-pipeline PROPERTIES LABELS "generic-pipeline;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(
    add_test(NAME pipeline4::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -n "1,3,1" -t "1,3,1" -R "(read,relayf,write)" -u 1)
    set_tests_properties(pipeline4::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline5::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -n "1,3,1" -t "1,3,1" -R "(read,relayf,write)" -u 1024)
    set_tests_properties(pipeline5::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # simple pipeline hybrid
    add_test(NAME pipeline6::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -t "1,3,1" -r "((read),(relayf,relay,relayf,relay),(write))")
    set_tests_properties(pipeline6::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline7::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -t "1,3,1" -r "((read),(relayf,relay,relayf,relay),(write))" -u 1024)
    set_tests_properties(pipeline7::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # pipeline multiple stages
    add_test(NAME pipeline8::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -t "1,3,1,3,1" -r "((read),(relayf,relay,relayf,relay),(relayf),(relay,relay),(write))")
    set_tests_properties(pipeline8::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline9::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -t "1,3,1,3,1" -r "((read),(relayf,relay,relayf,relay),(relayf),(relay,relay),(write))" -p)
    set_tests_properties(pipeline9::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline10::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -t "1,3,1,3,1" -r "((read),(relayf,relay,relayf,relay),(relayf),(relay,relay),(write))" -w)
    set_tests_properties(pipeline10::spu-test-generic-pipeline PROPERTIES LABELS "generic-pipeline;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(
    add_test(NAME pipeline11::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -t "1,3,1,3,1" -r "((read),(relayf,relay,relayf,relay),(relayf),(relay,relay),(write))" -u 1)
    set_tests_properties(pipeline11::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline12::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -t "1,3,1,3,1" -r "((read),(relayf,relay,relayf,relay),(relayf),(relay,relay),(write))" -u 1024)
    set_tests_properties(pipeline12::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # pipeline with socket type per stage
    add_test(NAME pipeline13::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -n "1,4,3,2,1" -t "1,3,1,3,1" -R "(read,relayf,relay,relayf,write)")
    set_tests_properties(pipeline13::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)

	############################# same tests with increment instead of relayers #######################################

    # simple sequence example
    add_test(NAME sequence4::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 100 -n "1,3,1" -t "3" -R "(init,incr,fin)" -q)
    set_tests_properties(sequence4::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME sequence5::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 100 -n "1,3,1" -t "3" -R "(init,incrf,fin)" -q -b)
    set_tests_properties(sequence5::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME sequence6::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 100 -t "3" -r "((init),(incrf,incr,incrf,incr),(fin))" -q)
    set_tests_properties(sequence6::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME sequence7::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 123 -t "1,3,1,3,1" -r "((init),(relayf,incr,relayf,relay),(relayf),(incr,relay),(fin))" -q -b)
    set_tests_properties(sequence7::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # simple pipeline example
    add_test(NAME pipeline14::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 1000 -n "1,3,1" -t "1,3,1" -R "(init,relay,fin)")
    set_tests_properties(pipeline14::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # simple pipeline FWD example
    add_test(NAME pipeline15::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 10 -n "1,3,1" -t "1,3,1" -R "(init,incrf,fin)")
    set_tests_properties(pipeline15::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline16::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 13 -n "1,3,1" -t "1,3,1" -R "(init,incrf,fin)" -p)
    set_tests_properties(pipeline16::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline17::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 9 -n "1,3,1" -t "1,3,1" -R "(init,incrf,fin)" -w)
    set_tests_properties(pipeline17::spu-test-generic-pipeline PROPERTIES LABELS "generic-pipeline;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(
    add_test(NAME pipeline18::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 13 -n "1,3,1" -t "1,3,1" -R "(init,relayf,fin)" -u 1)
    set_tests_properties(pipeline18::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline19::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 128 -n "1,3,1" -t "1,3,1" -R "(init,incrf,fin)" -u 1024)
    set_tests_properties(pipeline19::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # simple pipeline hybrid
    add_test(NAME pipeline20::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 128 -t "1,3,1" -r "((init),(relayf,relay,relayf,incr),(fin))")
    set_tests_properties(pipeline20::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline21::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 128 -t "1,3,1" -r "((init),(incrf,relay,relayf,incr),(fin))" -u 1024)
    set_tests_properties(pipeline21::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # pipeline multiple stages
    add_test(NAME pipeline22::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 12 -t "1,3,1,3,1" -r "((init),(relayf,relay,relayf,relay),(relayf),(incr,incr),(fin))")
    set_tests_properties(pipeline22::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline23::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 12 -t "1,3,1,3,1" -r "((init),(incrf,incr,incrf,incr),(incrf),(incr,incr),(fin))" -p)
    set_tests_properties(pipeline23::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline24::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 12 -t "1,3,1,3,1" -r "((init),(relayf,relay,relayf,relay),(incrf),(relay,relay),(fin))" -w)
    set_tests_properties(pipeline24::spu-test-generic-pipeline PROPERTIES LABELS "generic-pipeline;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(
    add_test(NAME pipeline25::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 12 -t "1,3,1,3,1" -r "((init),(relayf,incr,incrf,incr),(relayf),(incr,relay),(fin))" -u 1)
    set_tests_properties(pipeline25::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline26::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 12 -t "1,3,1,3,1" -r "((init),(incrf,relay,incrf,relay),(incrf),(relay,incr),(fin))" -u 1024)
    set_tests_properties(pipeline26::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    # pipeline with socket type per stage
    add_test(NAME pipeline27::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 12 -n "1,4,3,2,1" -t "1,3,1,3,1" -R "(init,incrf,incr,relayf,fin)")
    set_tests_properties(pipeline27::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)

    # scheduler tests
    add_test(NAME pipeline28::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 12 -t "1" -S "OTAC" -C "(init,relayf_15,incrementf_S_20,relay_15,fin)" -P "none")
    set_tests_properties(pipeline28::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline29::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 13 -t "4" -S "OTAC" -C "(init,relayf_15,incrementf_20,relay_15,fin)" -P "none")
    set_tests_properties(pipeline29::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline30::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 50 -t "3" -S "OTAC" -C "(init_S,relayf_S_15,incrementf_S_20,relay_S_15,fin_S)" -P "none")
    set_tests_properties(pipeline30::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline31::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 67 -t "6" -S "OTAC" -C "(init_S,relayf_S_15,incrementf_120,relay_S_15,fin_S)" -P "none")
    set_tests_properties(pipeline31::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline32::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 21 -t "3" -S "OTAC" -C "(init,relayf_15,incrementf_S_60,relay_15,fin)" -P "none")
    set_tests_properties(pipeline32::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)

    # m to n stages
    add_test(NAME pipeline33::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 10 -n "1,3,4,1" -t "1,3,4,1" -R "(init,incrf,incrf,fin)")
    set_tests_properties(pipeline33::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline34::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 13 -n "1,4,3,1" -t "1,4,3,1" -R "(init,incr,incr,fin)" -p -f 13)
    set_tests_properties(pipeline34::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline35::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 120 -t "1,3,4,5,1" -r "((init),(relayf,incr,incrf,incr),(relayf),(incr,relay),(fin))" -u 1)
    set_tests_properties(pipeline35::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)
    add_test(NAME pipeline36::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -e 120 -t "1,3,4,5,1" -r "((init),(relayf,incr,incrf,incr),(relayf),(incr,relay),(fin))" -u 1 -w)
    set_tests_properties(pipeline36::spu-test-generic-pipeline PROPERTIES LABELS "generic-pipeline;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(
    add_test(NAME pipeline37::spu-test-generic-pipeline COMMAND spu-test-generic-pipeline -i ${INPUT_FILE} -n "1,3,3,3,3,3,3,1" -t "1,3,1,4,7,32,1,1" -R "(read,relay,relayf,relay,relay,relay,relayf,write)")
    set_tests_properties(pipeline37::spu-test-generic-pipeline PROPERTIES LABELS generic-pipeline)

    add_test(NAME sequence0::spu-test-exclusive-paths-pipeline COMMAND spu-test-exclusive-paths-pipeline -t 1 -q -i ${INPUT_FILE})
    set_tests_properties(sequence0::spu-test-exclusive-paths-pipeline PROPERTIES LABELS exclusive-paths-pipeline)
    add_test(NAME sequence1::spu-test-exclusive-paths-pipeline COMMAND spu-test-exclusive-paths-pipeline -t 1 -u 13 -q -b -i ${INPUT_FILE})
    set_tests_properties(sequence1::spu-test-exclusive-paths-pipeline PROPERTIES LABELS exclusive-paths-pipeline)
    add_test(NAME pipeline0::spu-test-exclusive-paths-pipeline COMMAND spu-test-exclusive-paths-pipeline -t 4 -u 1 -i ${INPUT_FILE})
    set_tests_properties(pipeline0::spu-test-exclusive-paths-pipeline PROPERTIES LABELS exclusive-paths-pipeline)
    add_test(NAME pipeline1::spu-test-exclusive-paths-pipeline COMMAND spu-test-exclusive-paths-pipeline -t 1 -u 5 -i ${INPUT_FILE})
    set_tests_properties(pipeline1::spu-test-exclusive-paths-pipeline PROPERTIES LABELS exclusive-paths-pipeline)
    add_test(NAME pipeline2::spu-test-exclusive-paths-pipeline COMMAND spu-test-exclusive-paths-pipeline -t 8 -u 15 -i ${INPUT_FILE})
    set_tests_properties(pipeline2::spu-test-exclusive-paths-pipeline PROPERTIES LABELS exclusive-paths-pipeline)

    add_test(NAME sequence0::spu-test-nest-loops-pipeline COMMAND spu-test-nest-loops-pipeline -e 200 -f 2 -q)
    set_tests_properties(sequence0::spu-test-nest-loops-pipeline PROPERTIES LABELS nest-loops-pipeline)
    add_test(NAME sequence1::spu-test-nest-loops-pipeline COMMAND spu-test-nest-loops-pipeline -e 200 -f 13 -q -b -t 12)
    set_tests_properties(sequence1::spu-test-nest-loops-pipeline PROPERTIES LABELS nest-loops-pipeline)
    add_test(NAME pipeline0::spu-test-nest-loops-pipeline COMMAND spu-test-nest-loops-pipeline -e 10 -f 13 -i 10 -j 12 -t 8)
    set_tests_properties(pipeline0::spu-test-nest-loops-pipeline PROPERTIES LABELS nest-loops-pipeline)
    add_test(NAME pipeline1::spu-test-nest-loops-pipeline COMMAND spu-test-nest-loops-pipeline -e 10 -a -i 5 -j 13 -t 1)
    set_tests_properties(pipeline1::spu-test-nest-loops-pipeline PROPERTIES LABELS nest-loops-pipeline)
    add_test(NAME pipeline2::spu-test-nest-loops-pipeline COMMAND spu-test-nest-loops-pipeline -e 200 -i 5 -j 3 -t 2)
    set_tests_properties(pipeline2::spu-test-nest-loops-pipeline PROPERTIES LABELS nest-loops-pipeline)
    add_test(NAME pipeline3::spu-test-nest-loops-pipeline COMMAND spu-test-nest-loops-pipeline -e 200 -f 13)
    set_tests_properties(pipeline3::spu-test-nest-loops-pipeline PROPERTIES LABELS nest-loops-pipeline)

    # builder tests
    add_test(NAME pipeline0::spu-test-pipeline-builder-simple COMMAND spu-test-pipeline-builder-simple -i ${INPUT_FILE})
    set_tests_properties(pipeline0::spu-test-pipeline-builder-simple PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline1::spu-test-pipeline-builder-simple COMMAND spu-test-pipeline-builder-simple -i ${INPUT_FILE} -p)
    set_tests_properties(pipeline1::spu-test-pipeline-builder-simple PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline2::spu-test-pipeline-builder-simple COMMAND spu-test-pipeline-builder-simple -i ${INPUT_FILE} -w)
    set_tests_properties(pipeline2::spu-test-pipeline-builder-simple PROPERTIES LABELS "simple-pipeline;skip-memcheck") # to exclude this previous test from memchecks, active waiting is too long in Valgrind :-(
    add_test(NAME pipeline3::spu-test-pipeline-builder-simple COMMAND spu-test-pipeline-builder-simple -i ${INPUT_FILE} -u 1)
    set_tests_properties(pipeline3::spu-test-pipeline-builder-simple PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline4::spu-test-pipeline-builder-simple COMMAND spu-test-pipeline-builder-simple -i ${INPUT_FILE} -u 1024)
    set_tests_properties(pipeline4::spu-test-pipeline-builder-simple PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline5::spu-test-pipeline-builder-simple COMMAND spu-test-pipeline-builder-simple -i ${INPUT_FILE} -u 128 -o graph.dot)
    set_tests_properties(pipeline5::spu-test-pipeline-builder-simple PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline6::spu-test-pipeline-builder-simple COMMAND spu-test-pipeline-builder-simple -i ${INPUT_FILE} -u 8 -f 2)
    set_tests_properties(pipeline6::spu-test-pipeline-builder-simple PROPERTIES LABELS simple-pipeline)
    add_test(NAME pipeline7::spu-test-pipeline-builder-simple COMMAND spu-test-pipeline-builder-simple -i ${INPUT_FILE} -u 17 -f 5)
    set_tests_properties(pipeline7::spu-test-pipeline-builder-simple PROPERTIES LABELS simple-pipeline)

    add_test(NAME pipeline8::spu-test-pipeline-builder-conditional COMMAND spu-test-pipeline-builder-conditional)
    set_tests_properties(pipeline8::spu-test-pipeline-builder-conditional PROPERTIES LABELS simple-conditional)

    # hwloc tests
    if (SPU_LINK_HWLOC)
        # Static test to verify pinning (for CPUs with at least 4 hardware threads)
        add_test(NAME pipeline0::spu-test-thread-pinning COMMAND spu-test-thread-pinning -e 30)
        set_tests_properties(pipeline0::spu-test-thread-pinning PROPERTIES LABELS "thread-pinning;skip-memcheck") # to exclude this previous test from memchecks, it looks like there is a liitle memory leak in the hwloc lib but not sure
        add_test(NAME pipeline1::spu-test-thread-pinning COMMAND spu-test-thread-pinning -e 300 -f 5)
        set_tests_properties(pipeline1::spu-test-thread-pinning PROPERTIES LABELS "thread-pinning;skip-memcheck") # to exclude this previous test from memchecks, it looks like there is a liitle memory leak in the hwloc lib but not sure
    endif (SPU_LINK_HWLOC)

    # check if the automatic pipeline scheduling is giving expected results
    find_program (BASH_PROGRAM bash)
    if (BASH_PROGRAM)
        add_test (pipeline::sched ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/ci/test-sched.sh)
        set_tests_properties(pipeline::sched PROPERTIES LABELS "scheduler;skip-memcheck")
    endif (BASH_PROGRAM)
endif(SPU_TESTS)
