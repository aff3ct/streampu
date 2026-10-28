#ifdef SPU_JULIA

#include <atomic>
#include <cstddef>
#include <iostream>
#include <julia.h>
#include <mutex>
#include <sstream>

#include "Module/Stateless/Stateless_Julia.hpp"
#include "Runtime/Task/Task.hpp"
#include "Tools/Exception/exception.hpp"
#include "Tools/Julia/Julia.hpp"
#include "Tools/Julia/Proxy.hpp"
#include "Tools/Julia/Ref_keeper.hpp"

using namespace spu;
using namespace spu::module;

tools::Proxy
Stateless_Julia::jl_new_array_from_data(const runtime::Socket* sck, size_t n_elmts)
{
    void* p = sck->get_dataptr();
    // auto gc_state = jl_gc_enable(0);
    jl_value_t* datatype;

    if (sck->get_datatype() == typeid(uint8_t))
        datatype = (jl_value_t*)jl_uint8_type;
    else if (sck->get_datatype() == typeid(int8_t))
        datatype = (jl_value_t*)jl_int8_type;
    else if (sck->get_datatype() == typeid(uint16_t))
        datatype = (jl_value_t*)jl_uint16_type;
    else if (sck->get_datatype() == typeid(int16_t))
        datatype = (jl_value_t*)jl_int16_type;
    else if (sck->get_datatype() == typeid(uint32_t))
        datatype = (jl_value_t*)jl_uint32_type;
    else if (sck->get_datatype() == typeid(int32_t))
        datatype = (jl_value_t*)jl_int32_type;
    else if (sck->get_datatype() == typeid(uint64_t))
        datatype = (jl_value_t*)jl_uint64_type;
    else if (sck->get_datatype() == typeid(int64_t))
        datatype = (jl_value_t*)jl_int64_type;
    else if (sck->get_datatype() == typeid(float))
        datatype = (jl_value_t*)jl_float32_type;
    else if (sck->get_datatype() == typeid(double))
        datatype = (jl_value_t*)jl_float64_type;
    else
    {
        std::stringstream message;
        message << "Unsupported socket datatype ('sck->get_datatype_string()' = " << sck->get_datatype_string() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    tools::Proxy r = tools::Proxy((jl_value_t*)jl_ptr_to_array_1d(jl_apply_array_type(datatype, 1), p, n_elmts, 0));
    // jl_gc_enable(gc_state);
    return r;
}

size_t
Stateless_Julia::get_task_id(runtime::Task& task)
{
    auto& module = task.get_module();
    for (size_t tid = 0; tid < module.tasks.size(); tid++)
        if (&task == module.tasks[tid].get()) return tid;

    std::stringstream message;
    message << "Task is not in the current module, this should never happen.";
    throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
}

Stateless_Julia::Stateless_Julia(const bool jl_safe)
  : Module()
  , jl_constants_ptr(std::vector<std::vector<jl_value_t*>>())
  , jl_constants_id(std::vector<std::vector<size_t>>())
  , jl_func_args(std::vector<std::vector<void*>>())
  , evaluated(bool(false))
  , jl_eval_thread_keeper(nullptr)
  , n_clones(new size_t(0))
  , jl_create_constants(new std::vector<std::vector<std::function<void(Stateless_Julia& m)>>>())
  , jl_evaluate(new std::vector<std::function<void()>>())
  , jl_create_codelet(new std::vector<std::function<void(Stateless_Julia& m)>>())
  , jl_safe(jl_safe)
{
}

Stateless_Julia::~Stateless_Julia()
{
    if (this->jl_eval_thread_keeper != nullptr && this->jl_eval_thread_keeper == (void*)tools::Ref_keeper::get())
    {
        for (size_t tid = 0; tid < this->jl_constants_id.size(); tid++)
            for (size_t jl_id : this->jl_constants_id[tid])
                tools::Julia::release(jl_id);
    }
    (*this->n_clones)--;
}

Stateless_Julia*
Stateless_Julia::clone() const
{
    auto m = new Stateless_Julia(*this);
    m->deep_copy(*this);
    return m;
}

void
Stateless_Julia::deep_copy(const Stateless_Julia& m)
{
    Module::deep_copy(m);
    (*this->n_clones)++;
    this->reset();
}

void
Stateless_Julia::reset()
{
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
Stateless_Julia::is_eval() const
{
    return this->evaluated;
}

void
Stateless_Julia::set_jl_safety(const bool jl_safe)
{
    this->jl_safe = jl_safe;
}

bool
Stateless_Julia::is_jl_safe()
{
    return this->jl_safe;
}

void
Stateless_Julia::eval()
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
        message << "There is no Julia code to evaluate, please call the 'Stateless_Julia::create_codelet()' "
                   "method first.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    for (size_t tid = 0; tid < this->tasks.size(); tid++)
    {
        for (auto& create_constant : (*this->jl_create_constants)[tid])
            create_constant(*this);
        (*this->jl_evaluate)[tid]();
        (*this->jl_create_codelet)[tid](*this);
    }

    this->evaluated = true;
    this->jl_eval_thread_keeper = (void*)tools::Ref_keeper::get();
}

runtime::Task&
Stateless_Julia::create_task(const std::string& name)
{
    if ((*this->n_clones) > 0)
    {
        std::stringstream message;
        message << "It is not possible to create a new task when clones exist ('n_clones' = " << (*this->n_clones)
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->jl_create_constants->push_back(std::vector<std::function<void(Stateless_Julia & m)>>());
    this->jl_constants_ptr.push_back(std::vector<jl_value_t*>());
    this->jl_constants_id.push_back(std::vector<size_t>());
    this->jl_func_args.push_back(std::vector<void*>());

    return Module::create_task(name);
}

runtime::Task&
Stateless_Julia::create_tsk(const std::string& name)
{
    return this->create_task(name);
}

void
Stateless_Julia::create_codelet(runtime::Task& task, const std::string& julia_code)
{
    if ((*this->n_clones) > 0)
    {
        std::stringstream message;
        message << "It is not possible to create a new codelet when clones exist ('n_clones' = " << (*this->n_clones)
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->jl_evaluate->push_back([julia_code]() { jl_eval_string(julia_code.c_str()); });
    this->_create_codelet(task);
}

void
Stateless_Julia::create_cdl(runtime::Task& task, const std::string& julia_code)
{
    this->create_codelet(task, julia_code);
}

void
Stateless_Julia::create_codelet_file(runtime::Task& task, const std::string& julia_filepath)
{
    if ((*this->n_clones) > 0)
    {
        std::stringstream message;
        message << "It is not possible to create a new codelet when clones exist ('n_clones' = " << (*this->n_clones)
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->jl_evaluate->push_back([julia_filepath]() { tools::Julia::eval_file(julia_filepath); });
    this->_create_codelet(task);
}

void
Stateless_Julia::create_cdl_file(runtime::Task& task, const std::string& julia_filepath)
{
    this->create_codelet_file(task, julia_filepath);
}

// TODO: test
void
Stateless_Julia::create_codelet_function(runtime::Task& task, jl_function_t* f)
{
    if ((*this->n_clones) > 0)
    {
        std::stringstream message;
        message << "It is not possible to create a new codelet when clones exist ('n_clones' = " << (*this->n_clones)
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    size_t tid = Stateless_Julia::get_task_id(task);

    this->jl_evaluate->push_back(
      [this, f, tid]()
      {
          size_t f_id = tools::Julia::preserve(f);
          this->jl_constants_ptr[tid].push_back(f);
          this->jl_constants_id[tid].push_back(f_id);
      });
    this->_create_codelet(task);
}

void
Stateless_Julia::create_cdl_function(runtime::Task& task, jl_function_t* f)
{
    this->create_codelet_function(task, f);
}

void
Stateless_Julia::_create_codelet(runtime::Task& task)
{
    size_t tid = Stateless_Julia::get_task_id(task);

    // Shared compilation flag across the original task and all its thread clones
    auto task_compiled = std::make_shared<std::atomic<bool>>(false);

    Module::create_codelet(task,
                           [tid, task_compiled](module::Module& m, runtime::Task& t, const size_t frame_id)
                           {
                               auto& sjl = static_cast<Stateless_Julia&>(m);

                               if (!sjl.is_eval())
                               {
                                   std::stringstream message;
                                   message << "The task cannot be executed because the module has not been evaluated.";
                                   throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
                                   return 0;
                               }

                               std::vector<tools::Proxy> proxies;
                               auto gc_state = jl_gc_enable(0);
                               for (size_t s = 0; s < t.sockets.size() - 1; s++)
                               {
                                   const runtime::Socket* sck = t.sockets[s].get();
                                   sjl.jl_func_args[tid][s + 1] = (void*)Stateless_Julia::jl_new_array_from_data(
                                     sck, sck->get_n_elmts() * sjl.get_n_frames_per_wave() / sjl.get_n_frames());
                                   proxies.emplace_back((jl_value_t*)sjl.jl_func_args[tid][s + 1]);
                               }
                               jl_gc_enable(gc_state);

                               *(int32_t*)sjl.jl_func_args[tid][sjl.jl_func_args[tid].size() - 2] = frame_id;
                               *(int32_t*)sjl.jl_func_args[tid][sjl.jl_func_args[tid].size() - 1] =
                                 sjl.get_n_frames_per_wave();

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
                                       tools::Proxy proxy =
                                         tools::Julia::call(sjl.jl_func_args[tid], sjl.is_jl_safe());
                                       task_compiled->store(true, std::memory_order_release);
                                       return jl_unbox_int32((jl_value_t*)proxy);
                                   }
                               }

                               tools::Proxy proxy = tools::Julia::call(sjl.jl_func_args[tid], sjl.is_jl_safe());
                               return jl_unbox_int32((jl_value_t*)proxy);
                           });

    this->jl_create_codelet->push_back(
      [tid](Stateless_Julia& m)
      {
          runtime::Task& task = *(m.tasks[tid].get());
          if (m.jl_func_args[tid].size() > 0)
          {
              std::stringstream message;
              message << "jl_func_args[tid] has to be empty ('jl_func_args[tid].size()' = "
                      << m.jl_func_args[tid].size() << ", 'tid' = " << tid << ").";
              throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
          }

          m.jl_func_args[tid].resize(1 + task.sockets.size() + m.jl_constants_ptr[tid].size() + 1);

          if (m.jl_func_args[tid][0] == nullptr)
          {
              jl_function_t* jl_codelet = jl_get_function(jl_main_module, task.get_name().c_str());
              m.jl_func_args[tid][0] = (void*)jl_codelet;
          }

          for (size_t i = 0; i < m.jl_constants_ptr[tid].size(); i++)
              m.jl_func_args[tid][m.jl_func_args[tid].size() - (m.jl_constants_ptr[tid].size() + 2) + i] =
                (void*)m.jl_constants_ptr[tid][i];

          jl_value_t* jl_frame_id = jl_box_uint32(0);
          size_t jl_frame_id_id = tools::Julia::preserve(jl_frame_id);
          m.jl_constants_ptr[tid].push_back(jl_frame_id);
          m.jl_constants_id[tid].push_back(jl_frame_id_id);
          m.jl_func_args[tid][m.jl_func_args[tid].size() - 2] = (void*)jl_frame_id;

          jl_value_t* jl_n_frames_per_wave = jl_box_uint32(m.get_n_frames_per_wave());
          size_t jl_n_frames_per_wave_id = tools::Julia::preserve(jl_n_frames_per_wave);
          m.jl_constants_ptr[tid].push_back(jl_n_frames_per_wave);
          m.jl_constants_id[tid].push_back(jl_n_frames_per_wave_id);
          m.jl_func_args[tid][m.jl_func_args[tid].size() - 1] = (void*)jl_n_frames_per_wave;
      });
}

#endif /* SPU_JLUNA */
