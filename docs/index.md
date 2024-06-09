# StreamPU Documentation

## Introduction

`StreamPU` is a DSEL for streaming applications written in C++. This 
documentation maily focuses on explaining the basic elements of the language 
from the **developer point of view**. The user documention is is currently 
underway.

Here are the main features of `StreamPU`:

- Definition of modules, tasks and sockets (dataflow)
- Elementary modules and tasks implementations
- Parallel runtime (replication, pipeline)

The DSEL is suitable for SDR systems, video processing and more generally it 
matches **single-rate SDF streaming applications**.

!!! note
    This library was previously named `AFF3CT-core` as it was first extracted 
    from [AFF3CT](https://github.com/aff3ct/aff3ct). Now that it is no longer 
    specific to channel coding and digital communications, the project has been 
    renamed to `StreamPU`, which is a more meaningful name.

!!! note
    The **DSEL term** can be sometimes confusing and `StreamPU` can also be
    seen as a **standard C++ library**. The name "DSEL" comes from the ability 
    to write **interpreted Turing-complete programs** using the C++ library.

!!! warning
    This library is **NOT intended to address data and task parallelisms**.
    `StreamPU` focus on replication and pipeline parallelisms. For data 
    parallelism, `StreamPU` combines well with OpenMP. If you look for task
    parallelism, using OpenMP can also be a possible solution, or other runtime 
    like the excellent [StarPU](https://starpu.gitlabpages.inria.fr) can be a 
    good choice.

## Contents of the Documentation

**For Users**

  - Runtime:
    1. [Thread Pinning](thread_pinning.md)
  - Tests:
    1. [Bootstrap](tests_bootstrap.md)

**For Developpers**

 - Basic Components:
    1. [Task](task.md)
    2. [Module](module.md)
    3. [Socket](socket.md)
    4. [Sequence & Subsequence](sequence.md)
    5. [Pipeline](pipeline.md)  
    6. [Control flow](switcher.md)

 - New Features and Discussions:
    1. [Forward sockets](socket_fwd.md)
    2. [Pipeline & Control Flow](pipeline_ctrl_flow.md)
    3. [Work in Progress](wip.md)
