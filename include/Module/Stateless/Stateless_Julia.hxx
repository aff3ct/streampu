#include "Module/Stateless/Stateless_Julia.hpp"

namespace spu
{
namespace module
{
template<typename T>
void
Stateless_Julia::create_constant(runtime::Task& task, const T& value)
{
    if (this->cloned)
    {
        std::stringstream message;
        message << "It is not possible to create a new constant on a clone.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    for (auto s : task.sockets)
        if (s->get_name() == "status")
        {
            std::stringstream message;
            message << "Creating a constant after the codelet creation is forbidden.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

    size_t tid = Stateless_Julia::get_task_id(task);
    (*this->jl_create_constants)[tid].push_back(
      [tid, value](Stateless_Julia& m)
      {
          jluna::unsafe::Value* jl_constant_ptr = jluna::box<T>(value);

          // protect from garbage collector
          size_t jl_constant_id = jluna::unsafe::gc_preserve(jl_constant_ptr);

          m.jl_constants_ptr[tid].push_back(jl_constant_ptr);
          m.jl_constants_id[tid].push_back(jl_constant_id);
      });

    (*this->jl_create_constants)[tid][(*this->jl_create_constants).size() - 1](*this);
}
}
}