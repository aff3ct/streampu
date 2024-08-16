#ifdef SPU_JULIA

#include <sstream>

#include "Module/Stateless/Stateless_Julia.hpp"
#include "Tools/Exception/exception.hpp"

using namespace spu;
using namespace spu::module;

jluna::unsafe::Value*
Stateless_Julia::jl_call_func(std::vector<void*>& args)
{
    size_t n_args = args.size();
    if (n_args == 1)
    {
        return jluna::unsafe::call(static_cast<jluna::unsafe::Function*>(args[0]));
    }
    else if (n_args == 2)
    {
        return jluna::unsafe::call(static_cast<jluna::unsafe::Function*>(args[0]),
                                   static_cast<jluna::unsafe::Value*>(args[1]));
    }
    else if (n_args == 3)
    {
        return jluna::unsafe::call(static_cast<jluna::unsafe::Function*>(args[0]),
                                   static_cast<jluna::unsafe::Value*>(args[1]),
                                   static_cast<jluna::unsafe::Value*>(args[2]));
    }
    else if (n_args == 4)
    {
        return jluna::unsafe::call(static_cast<jluna::unsafe::Function*>(args[0]),
                                   static_cast<jluna::unsafe::Value*>(args[1]),
                                   static_cast<jluna::unsafe::Value*>(args[2]),
                                   static_cast<jluna::unsafe::Value*>(args[3]));
    }
    else if (n_args == 5)
    {
        return jluna::unsafe::call(static_cast<jluna::unsafe::Function*>(args[0]),
                                   static_cast<jluna::unsafe::Value*>(args[1]),
                                   static_cast<jluna::unsafe::Value*>(args[2]),
                                   static_cast<jluna::unsafe::Value*>(args[3]),
                                   static_cast<jluna::unsafe::Value*>(args[4]));
    }
    else if (n_args == 6)
    {
        return jluna::unsafe::call(static_cast<jluna::unsafe::Function*>(args[0]),
                                   static_cast<jluna::unsafe::Value*>(args[1]),
                                   static_cast<jluna::unsafe::Value*>(args[2]),
                                   static_cast<jluna::unsafe::Value*>(args[3]),
                                   static_cast<jluna::unsafe::Value*>(args[4]),
                                   static_cast<jluna::unsafe::Value*>(args[5]));
    }
    else
    {
        std::stringstream message;
        message << "Only n_args <= 6 is supported (n_args = '" << n_args << "').";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
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

Stateless_Julia::Stateless_Julia()
  : Module()
{
}

Stateless_Julia::~Stateless_Julia()
{
    // TODO: this is dangerous when a module is cloned, it should be done only by the last cloned module and not by all
    // of them
    for (size_t tid = 0; tid < this->jl_constants_id.size(); tid++)
        for (size_t jl_id : this->jl_constants_id[tid])
            jluna::unsafe::gc_release(jl_id);
}

Stateless_Julia*
Stateless_Julia::clone() const
{
    auto m = new Stateless_Julia(*this);
    m->deep_copy(*this);
    return m;
}

runtime::Task&
Stateless_Julia::create_task(const std::string& name)
{
    this->jl_constants_ptr.push_back(std::vector<jluna::unsafe::Value*>());
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
    jluna::Main.safe_eval(julia_code);
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
    jluna::Main.safe_eval_file(julia_filepath);
    this->_create_codelet(task);
}

void
Stateless_Julia::create_cdl_file(runtime::Task& task, const std::string& julia_filepath)
{
    this->create_codelet_file(task, julia_filepath);
}

void
Stateless_Julia::_create_codelet(runtime::Task& task)
{
    size_t tid = Stateless_Julia::get_task_id(task);

    if (this->jl_func_args[tid].size() > 0)
    {
        std::stringstream message;
        message << "jl_func_args[tid] has to be empty ('jl_func_args[tid].size()' = " << this->jl_func_args[tid].size()
                << ", 'tid' = " << tid << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->jl_func_args[tid].resize(1 + task.sockets.size() + this->jl_constants_ptr[tid].size());

    jluna::unsafe::Function* jl_codelet = jluna::unsafe::get_function(jluna::Main, jluna::Symbol(task.get_name()));

    this->jl_func_args[tid][0] = (void*)jl_codelet;

    for (size_t i = 0; i < this->jl_constants_ptr[tid].size(); i++)
        this->jl_func_args[tid][this->jl_func_args[tid].size() - jl_constants_ptr[tid].size() + i] =
          (void*)this->jl_constants_ptr[tid][i];

    Module::create_codelet(task,
                           [tid](module::Module& m, runtime::Task& t, const size_t frame_id) // TODO: forward 'frame_id'
                           {                                                                 // to the Julia function
                               auto& sjl = static_cast<Stateless_Julia&>(m);

                               for (size_t s = 0; s < t.sockets.size() - 1; s++)
                                   // TODO: be careful here, we need to adapt the jluna type to the real socket type!
                                   sjl.jl_func_args[tid][s + 1] = (void*)jluna::unsafe::new_array_from_data(
                                     jluna::UInt8_t, t.sockets[s]->get_dataptr(), t.sockets[s]->get_n_elmts());

                               jluna::unsafe::Value* jl_result = Stateless_Julia::jl_call_func(sjl.jl_func_args[tid]);

                               // return jluna::unbox<int32_t>(jl_result); // the unbox call is expensive!!
                               return jluna::unsafe::unsafe_unbox<int32_t>(jl_result); // this is cheap :-)
                           });
}

#endif /* SPU_JULIA */
