/*!
 * \file
 * \brief Class module::Module_Julia.
 */
#ifndef MODULE_JULIA_HPP_
#define MODULE_JULIA_HPP_

#ifdef SPU_JULIA

#include <atomic>
#include <cstdint>
#include <functional>
#include <julia.h>
#include <memory>
#include <string>
#include <vector>

#include "Module/Module.hpp"
#include "Runtime/Task/Task.hpp"
#include "Tools/Julia/Proxy.hpp"

namespace spu
{
namespace module
{
/*!
 * \class Module_Julia
 *
 * \brief Common Julia execution and state-management mixin shared by Stateless_Julia and Stateful_Julia.
 */
class Module_Julia
{
  protected:
    Module* self_module;
    Module_Julia& (*to_module_julia)(Module&);
    std::vector<jl_value_t*> jl_states_ptr;
    std::vector<size_t> jl_states_id;
    std::vector<std::vector<jl_value_t*>> jl_constants_ptr;
    std::vector<std::vector<size_t>> jl_constants_id;
    std::vector<std::vector<void*>> jl_func_args;
    bool evaluated;
    // Address of the thread_local Ref_keeper singleton that evaluated this module instance (set in eval()).
    // In multi-threaded Sequences and Pipelines, cloned Stateful_Julia / Stateless_Julia modules are evaluated on their
    // respective worker threads (allocating state and constant slot IDs in each worker's thread_local Ref_keeper),
    // whereas ~Sequence() always runs on the main thread and destroys all clones there. Tracking the owning Ref_keeper
    // ensures that ~Module_Julia() only calls tools::Julia::release(jl_id) when destroyed on the same thread that
    // evaluated it, preventing worker-thread slot IDs from corrupting the main thread's Ref_keeper freelist and marks
    // array.
    void* jl_eval_thread_keeper;
    std::shared_ptr<size_t> n_clones;

    std::shared_ptr<std::vector<std::function<void()>>> jl_evaluate_states;
    std::shared_ptr<std::vector<std::function<void(Module_Julia& m)>>> jl_create_states;
    std::shared_ptr<std::vector<std::vector<std::function<void(Module_Julia& m)>>>> jl_create_constants;
    std::shared_ptr<std::vector<std::function<void(Module_Julia& m)>>> jl_evaluate;
    std::shared_ptr<std::vector<std::function<void(Module_Julia& m)>>> jl_create_codelet;

    bool jl_safe;

  public:
    Module_Julia(Module* self_module, Module_Julia& (*to_module_julia)(Module&), const bool jl_safe = true);
    virtual ~Module_Julia();

    bool is_eval() const;

    void set_jl_safety(const bool jl_safe);
    bool is_jl_safe();

    template<typename T>
    void create_constant(runtime::Task& task, const T& value);

    void create_cdl(runtime::Task& task, const std::string& julia_code, const std::string& julia_function_name = "");
    void create_codelet(runtime::Task& task,
                        const std::string& julia_code,
                        const std::string& julia_function_name = "");

    void create_cdl_file(runtime::Task& task,
                         const std::string& julia_filepath,
                         const std::string& julia_function_name = "");
    void create_codelet_file(runtime::Task& task,
                             const std::string& julia_filepath,
                             const std::string& julia_function_name = "");

    void create_cdl_function(runtime::Task& task, jl_function_t* f);
    void create_codelet_function(runtime::Task& task, jl_function_t* f);

    void eval();

  protected:
    void deep_copy(const Module_Julia& m, Module* self_module);

    runtime::Task& _create_task(const std::string& name);

    void _create_codelet(runtime::Task& task, const std::string& julia_function_name = "");

    void _create_state(const std::string& julia_constructor,
                       const std::string& julia_code = "",
                       const bool shared = false);
    void _create_state_file(const std::string& julia_constructor,
                            const std::string& julia_filepath = "",
                            const bool shared = false);
    void _register_state(const std::string& julia_constructor, const bool shared);
    const std::vector<jl_value_t*>& _get_states() const;

    static size_t get_task_id(runtime::Task& task);
    static tools::Proxy jl_new_array_from_data(const runtime::Socket* sck, size_t n_elmts);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Module_Julia.hxx"
#endif

#endif /* SPU_JULIA */

#endif /* MODULE_JULIA_HPP_ */

