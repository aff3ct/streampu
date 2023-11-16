# AFF3CT-core

[![GitLab Pipeline Status](https://img.shields.io/gitlab/pipeline-status/aff3ct/aff3ct-core.svg?branch=development)](https://gitlab.com/aff3ct/aff3ct-core/pipelines)
[![GitLab Code Coverage](https://img.shields.io/gitlab/pipeline-coverage/aff3ct/aff3ct-core?branch=development)](https://aff3ct.gitlab.io/aff3ct-core/)
[![Documentation](https://img.shields.io/badge/doc-passing-green)](https://aff3ct.github.io/aff3ct-core/)
[![License: MIT](https://img.shields.io/github/license/aff3ct/aff3ct-core.svg)](./LICENSE)

`AFF3CT-core` is a **Domain Specific Embedded Language** (DSEL) for **streaming 
applications**. It comes in the form of a **C++11 library to link with**.

![Sequence and pipeline](./docs/assets/sequence_to_pipeline.svg)

Here are the main features of `AFF3CT-core`:
  - Definition of dataflow components: **modules**, **tasks** and **sockets**
  - Elementary modules and tasks implementations
  - **Multi-threaded runtime** with **replication** and **pipeline** parallel 
    constructs

This DSEL/library is suitable for SDR systems, audio/video processing and more 
generally it matches single-rate Synchronous DataFlow (SDF) streaming 
applications.

It is used as the multi-threaded runtime of 
[AFF3CT](https://github.com/aff3ct/aff3ct): a simulator for channel coding and a
library for real time Software-Defined Radio (SDR) systems.

## Linking with the Library

`AFF3CT-core` exposes two CMake targets:
- `aff3ct-core-shared-lib`: `AFF3CT-core` shared library, includes and 
  definitions
- `aff3ct-core-static-lib`: `AFF3CT-core` static library, includes and 
  definitions

For now, the recommended way to use `AFF3CT-core` is to add it as a 
`add_subdirectory` into your `CMakeList.txt` file. 

Let us suppose that we want to produce the `my-exe` executable that links with 
`AFF3CT-core` static library, here is a minimal `CMakeLists.txt` file to 
generate the `my-exe` executable:

```cmake
cmake_minimum_required(VERSION 3.5)

project(my_project CXX)

# require C++11 compiler ------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# create the `my-exe` executable from `src/main.cpp` --------------------------
add_executable(my-exe ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# compile `AFF3CT-core` static lib --------------------------------------------
option(AFF3CT_CORE_COMPILE_STATIC_LIB "" ON) # compile the static lib
option(AFF3CT_CORE_COMPILE_SHARED_LIB "" OFF) # do NOT compile the shared lib
option(AFF3CT_CORE_TESTS "" OFF) # do NOT compile the tests
# here we suppose that `AFF3CT-core` sources are located in `lib/aff3ct-core/`
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/aff3ct-core/)

# `my-exe` links with `AFF3CT-core` static lib --------------------------------
target_link_libraries(my-exe PUBLIC aff3ct-core-static-lib)
```

## Documentation

- [User documentation](https://largo.lip6.fr/~cassagnea/docs/UFR/MU5IN160/CM/CM5_AFF3CT_for_Streaming_Apps.pdf)
  * Slides used to teach `AFF3CT-core` to Master students in Computer Science at 
  [Sorbonne University](https://www.sorbonne-universite.fr/)
  * Contains many illustrative and simple use cases: **a good document to start 
    with `AFF3CT-core`**
- [Developer documentation](https://aff3ct.github.io/aff3ct-core/)
  * Online doc that focuses on how `AFF3CT-core` is built and works
  * Particularly **useful to learn how to modify `AFF3CT-core` and to 
    contribute**

## License

The project is licensed under the MIT license.

## How to cite AFF3CT-core

The main contributions of this work are described in the following journal 
article:  
- A. Cassagne, R. Tajan, O. Aumage, D. Barthou, C. Leroux and C. Jégo,  
  “**A DSEL for High Throughput and Low Latency Software-Defined Radio on Multicore CPUs**,“  
  *Wiley Concurrency and Computation: Practice and Experience (CCPE)*, 2023.  
  [[Open access article](https://doi.org/10.1002/cpe.7820)] [[Bibtex entry](https://aff3ct.github.io/resources/bibtex/Cassagne2023%20-%20A%20DSEL%20for%20High%20Throughput%20and%20Low%20Latency%20Software-Defined%20Radio%20on%20Multicore%20CPUs.bib)]

*To thank us even more*, we encourage you to also cite the original AFF3CT 
journal article to give more visibility to our work:  
- A. Cassagne, O. Hartmann, M. Léonardon, K. He, C. Leroux, R. Tajan, O. Aumage, D. Barthou, T. Tonnellier, V. Pignoly, B. Le Gal and C. Jégo,  
  “**AFF3CT: A Fast Forward Error Correction Toolbox!**,“  
  *Elsevier SoftwareX*, 2019.  
  [[Open access article](https://doi.org/10.1016/j.softx.2019.100345)] [[Bibtex entry](https://aff3ct.github.io/resources/bibtex/Cassagne2019a%20-%20AFF3CT:%20A%20Fast%20Forward%20Error%20Correction%20Toolbox.bib)]
