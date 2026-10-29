#ifdef SPU_JULIA

#include <atomic>
#include <cstdint>
#include <functional>
#include <julia.h>
#include <limits>
#include <mutex>
#include <sstream>
#include <typeindex>
#include <vector>

#include "Module/Module_Julia.hpp"
#include "Runtime/Socket/Socket.hpp"
#include "Tools/Exception/exception.hpp"
#include "Tools/Julia/Julia.hpp"
#include "Tools/Julia/Proxy.hpp"
#include "Tools/Julia/Ref_keeper.hpp"

using namespace spu;
using namespace spu::module;

tools::Proxy
Module_Julia::jl_new_array_from_data(const runtime::Socket* sck, size_t n_elmts)
{
    void* p = (void*)sck->get_dataptr();
    std::type_index type = sck->get_datatype();
    jl_value_t* datatype = nullptr;
    if (type == typeid(int8_t))
        datatype = (jl_value_t*)jl_int8_type;
    else if (type == typeid(int16_t))
        datatype = (jl_value_t*)jl_int16_type;
    else if (type == typeid(int32_t))
        datatype = (jl_value_t*)jl_int32_type;
    else if (type == typeid(int64_t))
        datatype = (jl_value_t*)jl_int64_type;
    else if (type == typeid(uint8_t))
        datatype = (jl_value_t*)jl_uint8_type;
    else if (type == typeid(uint16_t))
        datatype = (jl_value_t*)jl_uint16_type;
    else if (type == typeid(uint32_t))
        datatype = (jl_value_t*)jl_uint32_type;
    else if (type == typeid(uint64_t))
        datatype = (jl_value_t*)jl_uint64_type;
    else if (type == typeid(float))
        datatype = (jl_value_t*)jl_float32_type;
    else if (type == typeid(double))
        datatype = (jl_value_t*)jl_float64_type;
    else
    {
        std::stringstream message;
        message << "Unsupported socket datatype ('sck->get_datatype_string()' = " << sck->get_datatype_string() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    tools::Proxy r = tools::Proxy((jl_value_t*)jl_ptr_to_array_1d(jl_apply_array_type(datatype, 1), p, n_elmts, 0));
    return r;
}

size_t
Module_Julia::get_task_id(runtime::Task& task)
{
    auto& module = task.get_module();
    for (size_t tid = 0; tid < module.tasks.size(); tid++)
        if (&task == module.tasks[tid].get()) return tid;

    std::stringstream message;
    message << "Task is not in the current module, this should never happen.";
    throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
}

Module_Julia::Module_Julia(Module* self_module, Module_Julia& (*to_module_julia)(Module&), const bool jl_safe)
  : self_module(self_module)
  , to_module_julia(to_module_julia)
  , jl_states_ptr(std::vector<jl_value_t*>())
  , jl_states_id(std::vector<size_t>())
  , jl_constants_ptr(std::vector<std::vector<jl_value_t*>>())
  , jl_constants_id(std::vector<std::vector<size_t>>())
  , jl_func_args(std::vector<std::vector<void*>>())
  , evaluated(bool(false))
  , jl_eval_thread_keeper(nullptr)
  , n_clones(new size_t(0))
  , jl_evaluate_states(new std::vector<std::function<void()>>())
  , jl_create_states(new std::vector<std::function<void(Module_Julia& m)>>())
  , jl_create_constants(new std::vector<std::vector<std::function<void(Module_Julia& m)>>>())
  , jl_evaluate(new std::vector<std::function<void(Module_Julia& m)>>())
  , jl_create_codelet(new std::vector<std::function<void(Module_Julia& m)>>())
  , jl_safe(jl_safe)
{
}

Module_Julia::~Module_Julia()
{
    if (this->jl_eval_thread_keeper != nullptr && this->jl_eval_thread_keeper == (void*)tools::Ref_keeper::get())
    {
        for (size_t jl_id : this->jl_states_id)
            tools::Julia::release(jl_id);
        for (size_t tid = 0; tid < this->jl_constants_id.size(); tid++)
            for (size_t jl_id : this->jl_constants_id[tid])
                tools::Julia::release(jl_id);
    }
    (*this->n_clones)--;
}

void
Module_Julia::deep_copy(const Module_Julia& /*m*/, Module* self_module)
{
    this->self_module = self_module;
    (*this->n_clones)++;

    this->jl_states_ptr.clear();
    this->jl_states_id.clear();
    for (auto& ptrs_list : this->jl_constants_ptr)
        ptrs_list.clear();
    for (auto& ids_list : this->jl_constants_id)
        ids_list.clear();
    for (auto& funcs_list : this->jl_func_args)
        funcs_list.clear();

    this->evaluated = false;
    this->jl_eval_thread_keeper = nullptr;
}

bool
Module_Julia::is_eval() const
{
    return this->evaluated;
}

void
Module_Julia::set_jl_safety(const bool jl_safe)
{
    this->jl_safe = jl_safe;
}

bool
Module_Julia::is_jl_safe()
{
    return this->jl_safe;
}

const std::vector<jl_value_t*>&
Module_Julia::_get_states() const
{
    return this->jl_states_ptr;
}

void
Module_Julia::_create_state(const std::string& julia_constructor,
                            const bool shared,
                            const std::string& julia_code)
{
    if ((*this->n_clones) > 0)
    {
        std::stringstream message;
        message << "It is not possible to create a new state when clones exist ('n_clones' = " << (*this->n_clones)
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (!julia_code.empty())
    {
        auto code_evaluated = std::make_shared<std::atomic<bool>>(false);
        this->jl_evaluate_states->push_back(
          [julia_code, code_evaluated]()
          {
              if (!code_evaluated->exchange(true))
                  tools::Julia::eval_string(julia_code);
          });
    }

    this->_register_state(julia_constructor, shared);
}

void
Module_Julia::_create_state_file(const std::string& julia_constructor,
                                 const bool shared,
                                 const std::string& julia_filepath)
{
    if ((*this->n_clones) > 0)
    {
        std::stringstream message;
        message << "It is not possible to create a new state when clones exist ('n_clones' = " << (*this->n_clones)
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (!julia_filepath.empty())
    {
        auto file_evaluated = std::make_shared<std::atomic<bool>>(false);
        this->jl_evaluate_states->push_back(
          [julia_filepath, file_evaluated]()
          {
              if (!file_evaluated->exchange(true))
                  tools::Julia::eval_file(julia_filepath);
          });
    }

    this->_register_state(julia_constructor, shared);
}

void
Module_Julia::_register_state(const std::string& julia_constructor, const bool shared)
{
    if (shared)
    {
        auto shared_ptr = std::make_shared<jl_value_t*>(nullptr);
        this->jl_create_states->push_back(
          [julia_constructor, shared_ptr](Module_Julia& m)
          {
              if (*shared_ptr == nullptr)
              {
                  jl_value_t* jl_state_ptr = tools::Julia::eval_string(julia_constructor);
                  if (jl_state_ptr == nullptr)
                  {
                      std::stringstream message;
                      message << "Julia state constructor ('" << julia_constructor << "') returned nullptr.";
                      throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
                  }
                  size_t jl_state_id = tools::Julia::preserve(jl_state_ptr);
                  m.jl_states_id.push_back(jl_state_id);
                  *shared_ptr = jl_state_ptr;
              }
              m.jl_states_ptr.push_back(*shared_ptr);
          });
    }
    else
    {
        this->jl_create_states->push_back(
          [julia_constructor](Module_Julia& m)
          {
              jl_value_t* jl_state_ptr = tools::Julia::eval_string(julia_constructor);
              if (jl_state_ptr == nullptr)
              {
                  std::stringstream message;
                  message << "Julia state constructor ('" << julia_constructor << "') returned nullptr.";
                  throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
              }
              size_t jl_state_id = tools::Julia::preserve(jl_state_ptr);
              m.jl_states_ptr.push_back(jl_state_ptr);
              m.jl_states_id.push_back(jl_state_id);
          });
    }
}

void
Module_Julia::eval()
{
    static std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    if (this->is_eval())
    {
        std::stringstream message;
        message << "This module has already been evaluated.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if ((*this->jl_evaluate).size() == 0)
    {
        std::stringstream message;
        message << "There is no Julia code to evaluate, please call the 'create_codelet()' method first.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    // 1. Evaluate task files/strings and state type definitions in Main
    for (size_t tid = 0; tid < this->self_module->tasks.size(); tid++)
        (*this->jl_evaluate)[tid](*this);
    for (auto& eval_state_def : *this->jl_evaluate_states)
        eval_state_def();

    // 2. Instantiate and GC-preserve module states for this instance/clone
    for (auto& create_state : *this->jl_create_states)
        create_state(*this);

    // 3. Create task constants and bind jl_func_args (including module states at indices 1..K)
    for (size_t tid = 0; tid < this->self_module->tasks.size(); tid++)
    {
        for (auto& create_constant : (*this->jl_create_constants)[tid])
            create_constant(*this);
        (*this->jl_create_codelet)[tid](*this);
    }

    this->evaluated = true;
    this->jl_eval_thread_keeper = (void*)tools::Ref_keeper::get();
}

runtime::Task&
Module_Julia::_create_task(const std::string& name)
{
    if ((*this->n_clones) > 0)
    {
        std::stringstream message;
        message << "It is not possible to create a new task when clones exist ('n_clones' = " << (*this->n_clones)
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->jl_create_constants->push_back(std::vector<std::function<void(Module_Julia & m)>>());
    this->jl_constants_ptr.push_back(std::vector<jl_value_t*>());
    this->jl_constants_id.push_back(std::vector<size_t>());
    this->jl_func_args.push_back(std::vector<void*>());

    return this->self_module->create_task(name);
}

void
Module_Julia::create_codelet(runtime::Task& task,
                             const std::string& julia_code,
                             const std::string& julia_function_name)
{
    if ((*this->n_clones) > 0)
    {
        std::stringstream message;
        message << "It is not possible to create a new codelet when clones exist ('n_clones' = " << (*this->n_clones)
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->jl_evaluate->push_back([julia_code](Module_Julia&) { jl_eval_string(julia_code.c_str()); });
    this->_create_codelet(task, julia_function_name);
}

void
Module_Julia::create_cdl(runtime::Task& task,
                         const std::string& julia_code,
                         const std::string& julia_function_name)
{
    this->create_codelet(task, julia_code, julia_function_name);
}

void
Module_Julia::create_codelet_file(runtime::Task& task,
                                  const std::string& julia_filepath,
                                  const std::string& julia_function_name)
{
    if ((*this->n_clones) > 0)
    {
        std::stringstream message;
        message << "It is not possible to create a new codelet when clones exist ('n_clones' = " << (*this->n_clones)
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->jl_evaluate->push_back([julia_filepath](Module_Julia&) { tools::Julia::eval_file(julia_filepath); });
    this->_create_codelet(task, julia_function_name);
}

void
Module_Julia::create_cdl_file(runtime::Task& task,
                              const std::string& julia_filepath,
                              const std::string& julia_function_name)
{
    this->create_codelet_file(task, julia_filepath, julia_function_name);
}

void
Module_Julia::create_codelet_function(runtime::Task& task, jl_function_t* f)
{
    if ((*this->n_clones) > 0)
    {
        std::stringstream message;
        message << "It is not possible to create a new codelet when clones exist ('n_clones' = " << (*this->n_clones)
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    size_t tid = Module_Julia::get_task_id(task);

    this->jl_evaluate->push_back(
      [f, tid](Module_Julia& m)
      {
          size_t f_id = tools::Julia::preserve(f);
          m.jl_constants_ptr[tid].push_back(f);
          m.jl_constants_id[tid].push_back(f_id);
      });
    this->_create_codelet(task);
}

void
Module_Julia::create_cdl_function(runtime::Task& task, jl_function_t* f)
{
    this->create_codelet_function(task, f);
}

void
Module_Julia::_create_codelet(runtime::Task& task, const std::string& julia_function_name)
{
    size_t tid = Module_Julia::get_task_id(task);
    auto to_jl = this->to_module_julia;

    // Shared compilation flag across the original task and all its thread clones
    auto task_compiled = std::make_shared<std::atomic<bool>>(false);

    this->self_module->create_codelet(
      task,
      [tid, to_jl, task_compiled](module::Module& m, runtime::Task& t, const size_t frame_id) -> int
      {
          auto& sjl = to_jl(m);

          if (!sjl.is_eval())
          {
              std::stringstream message;
              message << "The task cannot be executed because the module has not been evaluated.";
              throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
              return 0;
          }

          const size_t n_states = sjl.jl_states_ptr.size();
          std::vector<tools::Proxy> proxies;
          auto gc_state = jl_gc_enable(0);
          for (size_t s = 0; s < t.sockets.size() - 1; s++)
          {
              const runtime::Socket* sck = t.sockets[s].get();
              sjl.jl_func_args[tid][1 + n_states + s] = (void*)Module_Julia::jl_new_array_from_data(
                sck, sck->get_n_elmts() * m.get_n_frames_per_wave() / m.get_n_frames());
              proxies.emplace_back((jl_value_t*)sjl.jl_func_args[tid][1 + n_states + s]);
          }
          jl_gc_enable(gc_state);

          *(uint32_t*)sjl.jl_func_args[tid][sjl.jl_func_args[tid].size() - 2] = (uint32_t)frame_id;
          *(uint32_t*)sjl.jl_func_args[tid][sjl.jl_func_args[tid].size() - 1] = (uint32_t)m.get_n_frames_per_wave();

          // If the Julia function has not been JIT compiled yet:
          if (!task_compiled->load(std::memory_order_acquire))
          {
              static std::mutex compile_mtx;
              int8_t prev_gc_state = 0;
              const bool can_safe = (jl_get_pgcstack() != nullptr);
              // Enter GC_SAFE state while waiting on the compilation lock
              // so that if another thread triggers a GC during JIT compilation,
              // this waiting thread will NOT block the Stop-The-World GC.
              if (can_safe)
                  prev_gc_state = jl_gc_safe_enter(jl_current_task->ptls);

              std::unique_lock<std::mutex> lock(compile_mtx);

              if (can_safe)
                  jl_gc_safe_leave(jl_current_task->ptls, prev_gc_state);

              if (!task_compiled->load(std::memory_order_relaxed))
              {
                  tools::Proxy proxy = tools::Julia::call(sjl.jl_func_args[tid], sjl.is_jl_safe());
                  task_compiled->store(true, std::memory_order_release);
                  return jl_unbox_int32((jl_value_t*)proxy);
              }
          }

          tools::Proxy proxy = tools::Julia::call(sjl.jl_func_args[tid], sjl.is_jl_safe());
          return jl_unbox_int32((jl_value_t*)proxy);
      });

    this->jl_create_codelet->push_back(
      [tid, julia_function_name](Module_Julia& m)
      {
          runtime::Task& task = *(m.self_module->tasks[tid].get());
          if (m.jl_func_args[tid].size() > 0)
          {
              std::stringstream message;
              message << "jl_func_args[tid] has to be empty ('jl_func_args[tid].size()' = "
                      << m.jl_func_args[tid].size() << ", 'tid' = " << tid << ").";
              throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
          }

          const size_t n_states = m.jl_states_ptr.size();
          const size_t n_csts = m.jl_constants_ptr[tid].size();
          m.jl_func_args[tid].resize(1 + n_states + task.sockets.size() + n_csts + 1);

          if (m.jl_func_args[tid][0] == nullptr)
          {
              const std::string& fn_name = julia_function_name.empty() ? task.get_name() : julia_function_name;
              jl_function_t* jl_codelet = jl_get_function(jl_main_module, fn_name.c_str());
              if (jl_codelet == nullptr)
              {
                  std::stringstream message;
                  message << "Julia function '" << fn_name << "' not found in Main module.";
                  throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
              }
              m.jl_func_args[tid][0] = (void*)jl_codelet;
          }

          for (size_t i = 0; i < n_states; i++)
              m.jl_func_args[tid][1 + i] = (void*)m.jl_states_ptr[i];

          for (size_t i = 0; i < n_csts; i++)
              m.jl_func_args[tid][m.jl_func_args[tid].size() - (n_csts + 2) + i] =
                (void*)m.jl_constants_ptr[tid][i];

          // Allocate dedicated heap-backed UInt32 boxes (passing a value >= 1024 bypasses Julia's
          // global box_uint32_cache[0..1023] singletons, which must never be mutated in-place).
          jl_value_t* jl_frame_id = jl_box_uint32(std::numeric_limits<uint32_t>::max());
          *(uint32_t*)jl_frame_id = 0;
          size_t jl_frame_id_id = tools::Julia::preserve(jl_frame_id);
          m.jl_constants_ptr[tid].push_back(jl_frame_id);
          m.jl_constants_id[tid].push_back(jl_frame_id_id);
          m.jl_func_args[tid][m.jl_func_args[tid].size() - 2] = (void*)jl_frame_id;

          jl_value_t* jl_n_frames_per_wave = jl_box_uint32(std::numeric_limits<uint32_t>::max());
          *(uint32_t*)jl_n_frames_per_wave = (uint32_t)m.self_module->get_n_frames_per_wave();
          size_t jl_n_frames_per_wave_id = tools::Julia::preserve(jl_n_frames_per_wave);
          m.jl_constants_ptr[tid].push_back(jl_n_frames_per_wave);
          m.jl_constants_id[tid].push_back(jl_n_frames_per_wave_id);
          m.jl_func_args[tid][m.jl_func_args[tid].size() - 1] = (void*)jl_n_frames_per_wave;
      });
}

#endif /* SPU_JULIA */

