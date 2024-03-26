# Thread pinning
`AFF3CT-core` offers a feature that allows users to choose on which `process unit (PU)` their threads will run using [Hwloc](https://www.open-mpi.org/projects/hwloc).
## Protable Hardware Locality (hwloc)
Hwloc is a software package which provides a **portable abstration** of the **hierarchical topology of modern architectures**, as we can see below for the orange pi 5 plus :  
![orange pi 5](./assets/hwloc_orangepi5.svg)

The idea behind using hwloc is its portability across Operating Systems, and the fact that it's not dependent on the architecture is running on. It also gives the ability to pin threads over any level of hierarchy with a tree view, where the process units are leaves and we have intern nodes which represent a set of `PU` that are physically close (share the same LLC or are in the same NUMA node).  
For example in the orange pi 5, we can choose to pin over a `Package` and the threads will run all over the process units that are within this level, if we choose `PACKAGE_0` the threads will run over the set of process units  `PU_0, PU_1, PU_3, PU_3`.  
The indexes given by hwloc are different from those given by the OS, this logical indexes express the real locality, so for the `AFF3CT-core` usage, it's important to use hwloc logical indexes.

## AFF3CT-core usage

The pinning can be used over `AFF3CT-core` sequences and pipelines, so we define a new simplified input syntax to express hwloc objects, a `std::string` with three different sperators :  
	- Pipeline stages :   `|`  
	- Threads of same stage (sequence)  : `;`  
    - Objects for the same thread : `,`  
  that we give as a parameter of pipeline or sequence constructors. We call this a pinning policy.
### Usage examples
 There are some examples to understand how the syntax works, we will use the simpliset hwloc object which is ```PU```. Supposing that we have a quad-core cpu with 4 process units (```PU_0, PU_1, PU_2, PU_3```), we want to describe a 3 stages pipeline with  :  
- One thread executing the first stage, we want to pin to ```PU_0```  
- Three threads executing the second, so that the first thread is pin to ```PU_0 & PU_1```, the second to ```PU_2``` and the last to ```PU_3```.  
-  One thread executing the last stage and we want to pin to ```PU_2 & PU_3```.   

The input parameters will be :   
-  The threads per stage vector : ```{1, 3, 1}```  
-  Enable pinning : `{true, true, true}`  
- The pinning policy : ```"PU_0 | PU_0, PU_1; PU_2; PU_3 | PU_2, PU3"```  

We can also support a packed input syntax if we want all the threads of the same stage to share a same hwloc object. We will add some complexity to our architecture, we suppose that  `PU_0 & PU_1` share the same LLC for data `L2D_0` and `PU_2 & PU_3` share another `L2D_1`, we can specify only one object for the second stage without replication for each thread :  
  - The pinning policy : ```"PU_0 | L2D_0 | PU_2, PU3"```  

Another level of syntax abstraction is possible for all the pipeline, all the threads of the pipeline will be executed on the specified objects :  
 - The pinning policy : ```"L2D_0, PU_3"```, all threads will be pinned to `PU_0, PU_1, PU_3`.  
  
  We have also the ability to choose the satges we want to pin using the vector of `boolean` enable pinning. For example if we dont want to pin the first stage we can :  
  -  Enable pinning : `{false, true, true}`  
  -  The pinning policy : ```"| L2D_0 | PU_2, PU3"```   
  
###Unpin

  An **unpin** function exists, it can be called by each thread individually, so that the calling thread will be free to be scheduled all over the process units.

## Disclaimer
We assume in our usage of thread pinning that the user knows the architecture hierarchy of his running machine, uses the logical indexes of hwloc and respects the syntaxe rules, otherwise the code will throw an exception.