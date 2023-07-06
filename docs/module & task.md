## Module  

#### Methods

```cpp
runtime::Task&  create_task(const  std::string  &name, const  int  id  =  -1)
```
Creates a new task, two tasks cannot share the same `name`
TODO tasks_with_nullptr


## Task  

#### Attributes  
`bool autoalloc`  TODO  
`bool stats` If true, records statistics regarding the task's execution, such as the`duration`  
`bool fast` If true, skips `can_exec()` check, improving runtime performance. Sockets bound to this task will also be set to `fast`  
`bool debug` If true, displays the task's sockets data and its status upon execution  
`bool debug_hex` If true, the data displayed during `debug` will be in hexadecimal  
`int32_t debug_limit` (?) Cf frames  
`uint8_t debug_precision`  
`int32_t debug_frame_max`  
`bool no_input_socket` If true, the task has no user-defined input sockets. See `bind()` for special behaviour.  
`std::function<int(module::Module  &m, Task& t, const  size_t frame_id)>  codelet` The anonymous function called by `_exec()`, thus dictating the task's behaviour. Usually set in the module's constructor, should return a `status_t`  
`std::shared_ptr<Socket> fake_input_socket` If the user does not create an input socket, a dummy input socket with no buffer is created to still allow binding even if the task does not require input data see `bind()` for special behaviour.  

#### Methods
```cpp
void reset()
```
Resets the task's statistics. Not to be confused with `Module::reset()`.
Manually called by the user.
```cpp
bool  can_exec() const
```
Returns true if every single of the task's socket was fed a buffer to read/write on. false otherwise. 
Called by `exec()` and skipped if `fast` is set to true.
```cpp
const  std::vector<int>&  exec(const  int  frame_id  =  -1, const  bool  managed_memory  =  true)
```
Calls `_exec()`and records execution statistics.

```cpp
void  _exec(const  int  frame_id  =  -1, const  bool  managed_memory  =  true)
```
Executes the task's `codelet` and sets the `status` for this specific call.
Called by `exec()`

```cpp
void  bind (Socket  &s_out, const  int  priority  =  -1);
```
Creates `fake_input_socket` and binds it to `s_out`. The new socket's `datatype` and `databytes` are matched to `s_out` for compatibility purposes during checks but they will not share any data during runtime. `fake_input_socket` is always `fast`.
Manually called by the user, regular sockets are directly bound with `Socket::bind()`
```cpp
size_t  unbind (Socket  &s_out )
```
Unbinds and deletes `fake_input_socket`.
Called by `Sequence::set_n_frames()` and manually by the user.


