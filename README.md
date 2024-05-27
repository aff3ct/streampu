# StreamPU

[![GitLab Pipeline Status](https://img.shields.io/gitlab/pipeline-status/aff3ct/streampu.svg?branch=develop)](https://gitlab.com/aff3ct/streampu/pipelines)
[![GitLab Code Coverage](https://img.shields.io/gitlab/pipeline-coverage/aff3ct/streampu?branch=develop)](https://aff3ct.gitlab.io/streampu/)
[![Documentation](https://img.shields.io/badge/doc-passing-green)](https://aff3ct.github.io/streampu/)
[![License: MIT](https://img.shields.io/github/license/aff3ct/streampu.svg)](./LICENSE)

`StreamPU` is a **Domain Specific Embedded Language** (DSEL) for **streaming 
applications**. It comes in the form of a **C++11 library to link with**.

![Sequence and pipeline](./docs/assets/sequence_to_pipeline.svg)

Here are the main features of `StreamPU`:
  - Definition of dataflow components: **modules**, **tasks** and **sockets**
  - Elementary modules and tasks implementations
  - **Multi-threaded runtime** with **replication** and **pipeline** parallel 
    constructs

This DSEL/library is suitable for SDR systems, audio/video processing and more 
generally it matches single-rate Synchronous DataFlow (SDF) streaming 
applications.

Please note that this library was previously named `AFF3CT-core` as it was first 
extracted from [AFF3CT](https://github.com/aff3ct/aff3ct). Now that it is no 
longer specific to channel coding and digital communications, the project has 
been renamed to `StreamPU`, which is a more meaningful name.

## Library Compilation & Installation

This project uses CMake as the main build system and it provides the following
CMake options:
- `SPU_COMPILE_STATIC_LIB`: Compile the static library (default = `ON`)
- `SPU_COMPILE_SHARED_LIB`: Compile the shared library (default = `OFF`)
- `SPU_LINK_HWLOC`: Link with the `hwloc` library (used for threads pinning) 
  (default = `OFF`)
- `SPU_COLORS`: Enable the colors in the terminal (default = `ON`)
- `SPU_TESTS`: Enable the compilation of the tests (default = `ON`)
- `SPU_STACKTRACE`: Print the stack trace when an exception is raised (and link 
  with the `cpptrace` lib) (default = `ON`)
- `SPU_STACKTRACE_SEGFAULT`: Try to print the stack trace when a segfault occurs 
  (and link with the `cpptrace` lib) (default = `OFF`)
- `SPU_SHOW_DEPRECATED`: Print message each time a deprecated function is called 
  (default = `OFF`)
- `SPU_FAST`: Remove checks to speedup the code (default = `OFF`)

Build the library in `debug` mode:
```bash
mkdir build_debug
cd build_debug
cmake ..
make -j4
ctest
```

Build the library in `release` mode:
```bash
mkdir build_release
cd build_release
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-Wall -march=native -funroll-loops"
make -j4
ctest
```

Install the library on the system (`/usr/local`):
```bash
cmake --install .
```

Install the library in a custom path (`/opt/streampu`):
```bash
cmake --install . --prefix /opt/streampu/
```

## Link with the Library

`StreamPU` exposes two CMake targets:
- `spu::spu-static-lib`: `StreamPU` static library, includes and definitions 
  (compiled by default)
- `spu::spu-shared-lib`: `StreamPU` shared library, includes and definitions

It is possible to link with `StreamPU` from a sub-directory or from an installed
library. Both methods are described in the next sections.

### From a Sub-directory

Let us suppose that we want to produce the `my-exe` executable that will links 
with `StreamPU` and that the `StreamPU` repository is located at 
`my-exe-root/lib/streampu/`. Here is a minimal `CMakeLists.txt` file to 
generate it:

```cmake
cmake_minimum_required(VERSION 3.5)

project(my_project CXX)

# require C++11 compiler ------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# create the `my-exe` executable from `src/main.cpp` --------------------------
add_executable(my-exe ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# compile `StreamPU` static lib -----------------------------------------------
option(SPU_TESTS "" OFF) # do NOT compile the tests
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/streampu/)

# `my-exe` links with `StreamPU` static lib -----------------------------------
target_link_libraries(my-exe PUBLIC spu::spu-static-lib)

# `my-exe` links with `cpptrace` (`SPU_STACKTRACE=ON`) ------------------------
if(SPU_STACKTRACE OR SPU_STACKTRACE_SEGFAULT)
  target_link_libraries(my-exe PUBLIC cpptrace::cpptrace)
endif()
```

### From an Installed Library

Let us suppose that we want to produce the `my-exe` executable that links with 
`StreamPU` static library installed on the system. Here is a minimal 
`CMakeLists.txt` file to generate it:

```cmake
cmake_minimum_required(VERSION 3.5)

project(my_project CXX)

# require C++11 compiler ------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# create the `my-exe` executable from `src/main.cpp` --------------------------
add_executable(my-exe ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# find and link with `cpptrace` if installed (if not, don't) ------------------
find_package(cpptrace CONFIG 0.4.1)
if (cpptrace_FOUND)
  target_link_libraries(my-exe PUBLIC cpptrace::cpptrace)
endif()

# find and link with `StreamPU` -----------------------------------------------
find_package(StreamPU CONFIG 0.1.0 REQUIRED)
target_link_libraries(my-exe PUBLIC spu::spu-static-lib)
```

If `StreamPU` is installed in a specific location (for instance: 
`/opt/streampu`), you can help CMake to find it as follow:
```bash
cmake .. -DStreamPU_DIR=/opt/streampu/lib/cmake/streampu
```

Or, if StreamPU has been compiled with the `cpptrace` library:
```bash
cmake .. -DStreamPU_DIR=/opt/streampu/lib/cmake/streampu -Dcpptrace_DIR=/opt/streampu/lib/cmake/cpptrace
```

## Documentation

- [User documentation](https://largo.lip6.fr/~cassagnea/docs/UFR/MU5IN160/CM/CM5_AFF3CT_for_Streaming_Apps.pdf)
  * Slides used to teach `StreamPU` to Master students in Computer Science at 
  [Sorbonne University](https://www.sorbonne-universite.fr/)
  * Contains many illustrative and simple use cases: **a good document to start 
    with `StreamPU`**
- [Developer documentation](https://aff3ct.github.io/streampu/)
  * Online doc that focuses on how `StreamPU` is built and works
  * Particularly **useful to learn how to modify `StreamPU` and to contribute**

## License

The project is licensed under the MIT license.

## How to cite `StreamPU`

The main contributions of this work are described in the following journal 
article:  
- A. Cassagne, R. Tajan, O. Aumage, D. Barthou, C. Leroux and C. Jégo,  
  “**A DSEL for High Throughput and Low Latency Software-Defined Radio on Multicore CPUs**,“  
  *Wiley Concurrency and Computation: Practice and Experience (CCPE)*, 2023.  
  [[Open access article](https://doi.org/10.1002/cpe.7820)] [[Bibtex entry](https://aff3ct.github.io/resources/bibtex/Cassagne2023%20-%20A%20DSEL%20for%20High%20Throughput%20and%20Low%20Latency%20Software-Defined%20Radio%20on%20Multicore%20CPUs.bib)]
