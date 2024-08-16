#include "Module/Stateless/Stateless_Julia.hpp"

namespace spu
{
namespace module
{
template<typename T>
void
Stateless_Julia::create_constant(runtime::Task& task, const T& value)
{
    for (auto s : task.sockets)
        if (s->get_name() == "status")
        {
            std::stringstream message;
            message << "Creating a constant after the codelet creation is forbidden.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

    size_t tid = Stateless_Julia::get_task_id(task);

    jluna::unsafe::Value* jl_constant_ptr = jluna::box<T>(value);

    // protect from garbage collector
    size_t jl_constant_id = jluna::unsafe::gc_preserve(jl_constant_ptr);

    this->jl_constants_ptr[tid].push_back(jl_constant_ptr);
    this->jl_constants_id[tid].push_back(jl_constant_id);
}
}
}