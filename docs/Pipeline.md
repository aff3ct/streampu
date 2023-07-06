## Pipeline
Pipeline is a mechanism offered by `AFF3CT-core` to execute a [Sequence](Sequence.md), by dividing it into multiple stages and execute each stage on one or multiple threads. 

		Mettre image pour illustrer le passage de séquence a pipeline !!

### Attributes 
`Sequence  original_sequence` : The original sequence from witch the pipeline is created.  
`std::vector<std::shared_ptr<Sequence>>  stages `: Vector of the different stages (each stage is a sequence) of the pipeline.  
```cpp
std::vector<std::pair<
	std::tuple<runtime::Socket*, size_t, size_t, size_t,size_t>,  
	std::tuple<runtime::Socket*, size_t, size_t, size_t>>>  sck_orphan_binds : 
```
Vector of sockets with broken connections due to pipeline stage creation, these sockets will be bound later to special modules called [Adaptors](#Adaptor), to connect the different stages.  

```cpp
std::vector<std::tuple<runtime::Socket*, runtime::Socket*, size_t>>  adaptors_binds 
```
Vector of tuple (input, output, priority) for the created adaptors, priority is used to order the tuples.

### Methods 



### Adaptor 
