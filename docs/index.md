# AFF3CT-core Developer Documentation

## Introduction

`AFF3CT-core` is a DSEL for streaming applications written in C++. This 
documentation focus on explaining the basic elements of the language from the 
**developer point of view**.

Here are the main features of `AFF3CT-core`:

- Definition of modules, tasks and sockets (dataflow)
- Elementary modules and tasks implementations
- Parallel runtime (replication, pipeline)

The DSEL is suitable for SDR systems, video processing and more generally it 
matches **single-rate SDF streaming applications**.

!!! note
    The **DSEL term** can be sometimes confusing and `AFF3CT-core` can also be
    seen as a **standard C++ library**. The name "DSEL" comes from the ability 
    to write **interpreted Turing-complete programs** using the C++ library.

!!! warning
    This library is **NOT intended to address data and task parallelisms**.
    `AFF3CT-core` focus on replication and pipeline parallelisms. For data 
    parallelism, `AFF3CT-core` combines well with OpenMP. If you look for task
    parallelism, using OpenMP can also be a possible solution, or other runtime 
    like the excellent [StarPU](https://starpu.gitlabpages.inria.fr) can be a 
    good choice.

## Contents of the Documentation

Basic components:

1. [Task](task.md)
2. [Module](module.md)
3. [Socket](socket.md)
4. [Sequence & Subsequence](sequence.md)
5. [Pipeline](pipeline.md)  
6. [Control flow](switcher.md)

New features and discussions:

1. [Forward sockets](socket_fwd.md)
2. [Pipeline & Control Flow](pipeline_ctrl_flow.md)
3. [Work in Progress](wip.md)

Tests:

1. [Bootstrap](tests_bootstrap.md)