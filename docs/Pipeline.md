## Pipeline

Pipeline is a mechanism offered by `AFF3CT-core` to execute a [Sequence](Sequence.md), by dividing it into multiple stages and execute each stage on one or multiple threads.

  

![text alternatif](/images/pipeline_interstage_fwd.pdf "Test insertion image")

### Attributes

`Sequence original_sequence` : The original sequence from which the pipeline is created.

`std::vector<std::shared_ptr<Sequence>> stages `: Vector of the different stages (each stage is a sequence) of the pipeline.

```cpp

std::vector<std::pair<

std::tuple<runtime::Socket*, size_t, size_t, size_t,size_t>,

std::tuple<runtime::Socket*, size_t, size_t, size_t>>> sck_orphan_binds :

```

Vector of sockets with broken connections due to pipeline stage creation, these sockets will be bound later to special modules called [Adaptors](#Adaptors), to connect the different stages.

  

```cpp

std::vector<std::tuple<runtime::Socket*, runtime::Socket*, size_t>> adaptors_binds

```

Vector of tuple (input, output, priority) for the created adaptors, priority is used to order the tuples.

<<<<<<< HEAD
  
=======
### Methods 
```cpp
void  init(const  std::vector<TA*> &firsts,
		   const  std::vector<TA*> &lasts,
		   const  std::vector<std::tuple<std::vector<TA*>, std::vector<TA*>, std::vector<TA*>>> &sep_stages  = {},
		   const  std::vector<size_t> &n_threads  = {},
		   const  std::vector<size_t> &synchro_buffer_sizes  = {},
		   const  std::vector<bool> &synchro_active_waiting  = {},
		   const  std::vector<bool> &thread_pinning  = {},
		   const  std::vector<std::vector<size_t>> &puids  = {} );
```
This function builds the pipeline given :

 - The first task of the original sequence.
 - The firsts and lasts tasks of each stage.
 - The number of threads to execute on eache stage.
 - The number of buffers between stages.

```cpp
void  create_adaptors(const  std::vector<size_t> &synchro_buffer_sizes  = {},
					  const  std::vector<bool> &synchro_active_waiting  = {});
```
This function creates the adaptor tasks that are added between every stage to transmit data from the stage $(S)$ to stage $S+1$.  AFF3CT don't support two multi-threaded stages following each other.

```cpp
void  _bind_adaptors(const  bool  bind_adaptors  =  true);
```
Adaptor module tasks need to be bound to each task in the two consecutive stages, the target sockets to bind are stored in the vector`sck_orphan_binds.`
>>>>>>> 287d61925210fb7a6c3bbe5ef81ab530fa70664c

### Methods

```cpp

<<<<<<< HEAD
void  init(const  std::vector<TA*> &firsts,

 const  std::vector<TA*> &lasts,

 const  std::vector<std::tuple<std::vector<TA*>, std::vector<TA*>, std::vector<TA*>>> &sep_stages = {},

 const  std::vector<size_t> &n_threads = {},

 const  std::vector<size_t> &synchro_buffer_sizes = {},

 const  std::vector<bool> &synchro_active_waiting = {},

 const  std::vector<bool> &thread_pinning = {},

 const  std::vector<std::vector<size_t>> &puids = {} );

```

This function builds the pipeline given :

  

- The first task of the original sequence.

- The firsts and lasts tasks of each stage.

- The number of threads to execute on each stage.

- The number of buffers between stages.

  

```cpp

void  create_adaptors(const  std::vector<size_t> &synchro_buffer_sizes = {},

 const  std::vector<bool> &synchro_active_waiting = {})

```

This function creates the adaptor tasks that are added between every stage to transmit data from the stage $(S)$ to stage $S+1$. AFF3CT don't support two multithreaded stages following each other.

  

```cpp

void  _bind_adaptors(const  bool  bind_adaptors = true)

```

Adaptor module tasks need to be bound to each task in the two consecutive stages, the target sockets to bind are stored in the vector`sck_orphan_binds.`

  
  
  

### Adaptors

  

Adaptors are special modules added by the application at runtime when creating a pipeline. As said before, the adaptors are bound to edge tasks between each two consecutive stages. The purpose of adaptors is to synchronize data exchange between each stage. We have 4 tasks performed by adaptor :

  

- push_1 : when the $S$ stage is executed on one thread and the $S+1$ stage is on multiple threads, the function pushes data to every interstage buffer of each thread using a round-robin algorithm.

- push_n : when the $S$ stage is executed on multiple threads and the $(S-1)$ stage is on one thread, it's the task that faces the `push_1`, it takes data from the interstage buffer and forward it to the first task of each thread.

- push_n : when the $S$ stage is executed on multiple threads and the $(S+1)$ stage is on one thread, the task gathers the data from each thread last task output buffer and pass it to the first task on the stage, it uses the same algorithm of the `push_1` function.

- pull_1 : when the $S$ stage is executed on one thread and the $(S-1)$ stage is on multiple threads, it's the task that faces the `push_n`, it forwards data to the real first task of the stage.
=======
## Adaptor 

Adaptors are special modules added by the application when creating a pipeline. As said before, the adaptors are binded to edge tasks between each two consecutive stages. The purpose of adaptors is to synchronise data exchange between each stage. We have 4 tasks performed by adaptor : 

 - push_1 : when the $S$ stage is executed on one thread and the $S+1$ stage is on multiple threads, the function pushes data to every interstage buffer of each thread using a round robin algorithm. 
  - push_n : when the $S$ stage is executed on multiple threads and the $(S-1)$ stage is on one thread, it's the task that faces the `push_1`, it takes data from the interstage buffers and forward it to the first task of each thread.
  - push_n :   when the $S$ stage is executed on multiple threads and the $(S+1)$ stage is on one thread, the task gathers the data from each thread last task output buffer and pass it to the first task on the stage, it uses the same algorithm of the `push_1` function.
  - pull_1 :  when the $S$ stage is executed on one thread and the $(S-1)$ stage is on multiple threads, it's the task that faces the `push_n`, it forwards data to the real first task of the stage.

### Attributs

`std::vector<size_t>  n_elmts `: ?
`const  size_t  buffer_size ` : The interstage buffer pool size.
`std::shared_ptr<std::vector<std::vector<std::vector<int8_t*>>>>  buffer : Pointers to each buffer`of the interstage pool.
``` cpp 
std::shared_ptr<std::vector<std::atomic<uint64_t>>>  first;
std::shared_ptr<std::vector<std::atomic<uint64_t>>>  last;
```
Two pointers used to monitor the buffer pool ? 

### Methods





 
>>>>>>> 287d61925210fb7a6c3bbe5ef81ab530fa70664c
