# Overview

## Tasks, Modules & Sockets

TODO.

## Sequences and Pipelines

TODO.

## Module States & Tasks Replicability

In StreamPU, there are modules that are either `stateful` or `stateless`. 
`stateful` means that it is an instance of a class that inherits from 
`module::Stateful` class. On the other hand, a `stateless` module is necessarily 
an instance of the `module::Stateless` class. The `module::Stateful` and 
`module::Stateless` classes both inherit directly from the `module::Module` 
class. All `stateless` modules are `clonable`. This means that, by default, 
`stateless` module tasks are `replicable` (i.e. they can be replicated within a 
sequence or pipeline stage to increase the system throughput). 

It gets a little bit more complex with `stateful` modules. By default, a module 
of this type is `non-clonable` (i.e. it does not implement the 
`module::Stateful::clone()` method). In this case, the tasks that make up the 
module are, by default, `non-replicable`. However, the designer of a `stateful` 
module can override the `module::Stateful::clone()` method (basically, the 
designer has to expresses how to "copy" the module). When it is done, the module 
becomes `clonable` and, automatically, the tasks that make up a `clonable` 
module become `replicable`.

Sometimes one may want to force a `replicable` task to avoid being replicated. 
For instance, 1) for testing purposes or 2) sometimes certain tasks could be 
replicated, but in the way they are used it makes no sense and would lead to 
application malfunction. For this purpose, the public
`runtime::Task::set_replicability(bool)` method gives control to the designer.
If the boolean is set to `false`, then the task is forced to be 
`non-replicable`.

!!! warning
    If the input boolean of the `runtime::Task::set_replicability(bool)` method 
    is set to `true` and the corresponding module is `non-clonable`, the method 
    will fail and will throw an exception. In any cases, it will be possible to 
    set the boolean to `false` (who can do more can do less).

To summarize:

- `stateless` module:
    * Always `clonable`
    * Tasks are by default `replicable`
    * Tasks can be forced to be `non-replicable` by the designer

- `stateful` module:
    * By default `non-clonable`
    * Designer can implement the `module::Stateful::clone()` method to make the 
      module `clonable`
    * By default, if the `module::Stateful::clone()` method is not implemented,
      tasks are `non-replicable`
    * If the `module::Stateful::clone()` method is implemented, tasks become
      `replicable` and the designer can set the replicability to `true` or 
      `false`

