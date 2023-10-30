# Task

A **task** represents **the code executed by a node in the data flow graph**. In 
other languages, a **task** can be refereed as a *job* or a *filter*. 
A task is defined by its input and output data and the code to execute when 
triggered. In other word, a task comes with a set of data called 
[sockets](socket.md) (not to be confused with network and system sockets). The 
sockets model the data that are consumed (input socket) and produced (output 
socket) by the current task. Finally, the code to execute is stored in a 
so-called *codelet*.

A task is a C++ object of the `aff3ct::runtime::Task` class. The following
sections try to give an overview of the most important attributes and methods
to facilitate the code understanding.

## Main Attributes

```cpp
using namespace aff3ct;
/* 
 * The list of sockets that are attached to this task.
 */
std::vector<std::shared_ptr<runtime::Socket>> sockets;
/* 
 * The socket types corresponding to the previous `sockets` attribute, in the 
 * same order. Can be `socket_t::SIN`, `socket_t::SOUT` or `socket_t::SFWD` for 
 * input socket, output socket and forward socket, respectively.
 */
std::vector<runtime::socket_t> socket_type;
/* 
 * The allocated data of the output sockets of this task. If the `autoalloc` 
 * attribute is set to `True` (see below) then the data are allocated here, 
 * otherwise this vector is left empty.
 */
std::vector<std::vector<uint8_t>> out_buffers;
/* 
 * The function called by `_exec()` method (see below), thus dictating the 
 * task's behavior. Usually set in the module's constructor, should return a 
 * `status_t`.
 */
std::function<int(module::Module &m, runtime::Task& t, const size_t frame_id)> 
    codelet;
/* 
 * Fake input sockets are used when specifying dependencies between tasks
 * directly. Thus, in intern, these dependencies are managed through "fake input
 * sockets" that are created on-the-fly over the current task. The data of these
 * sockets are ignored during the codelet execution.
 */
std::shared_ptr<runtime::Socket> fake_input_sockets;
/*
 * If set to `True`, let `AFF3CT-core` allocate and reallocate memory needed by 
 * the task. Data are only allocated in the output sockets. By default this 
 * attribute is set to `True`.
 */
bool autoalloc;
/*
 * If true, records statistics regarding the task's execution, such as the 
 * `duration`. By default this attribute is set to `False`.
 */
bool stats;
/* 
 * If true, skips `can_exec()` runtime check, thus, improving performance. 
 * Sockets bound to this task will also be set to `fast`. By default this 
 * attribute is set to `False`.
 */
bool fast;
/*
 * If set to true, displays the task's sockets data and its status upon 
 * execution (on the standard output). By default this attribute is set to 
 * `False`.
 */
bool debug;
/*
 * A pointer to the corresponding Module. See bellow for more information about
 * what is precisely a Module.
 */
module::Module *module;
/*
 * A name to identify the task in the Module. This name is unique in the Module.
 */
const std::string name;
```


## Main Methods

```cpp
using namespace aff3ct;
/*
 * Calls `_exec()` method, records execution statistics (if `stats == True`) and 
 * prints the debug logs (if `debug == True`).
 */
const std::vector<int>& exec(const int frame_id = -1, 
                             const bool managed_memory = true);
/*
 * Executes the task's `codelet` and sets the `status` for this specific call.
 * Called by `exec()` (see the above method).
 */
void _exec(const int frame_id = -1, const bool managed_memory = true);
/*
 * Returns `True` if all the sockets are associated to an allocated buffer, 
 * otherwise returns `False`. Called by `exec()` method if `fast` is set to 
 * `True`, skipped otherwise.
 */
bool can_exec() const;
/*
 * Add a fake input socket to the current task (see above `fake_input_sockets` 
 * attribute) and binds it to the output `status` socket of the `t_out` task in 
 * parameter. The new socket's `datatype` and `databytes` matches the output 
 * `status` socket of `t_out`. `fake_input_sockets` is always `fast`. This 
 * method has to be manually called by the user.
 */
void bind(runtime::Task &t_out, const int priority = -1);
/*
 * Unbinds and deletes the corresponding input socket in the 
 * `fake_input_sockets` attribute. Can be called by `Sequence::set_n_frames()` 
 * or manually by the user.
 */
size_t unbind(runtime::Task &t_out);
/* 
 * Resets the task's statistics. Not to be confused with `Module::reset()`.
 * Manually called by the user.
 */
void reset();
```
