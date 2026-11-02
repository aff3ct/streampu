#include "Module/Module_Julia.hpp"
#include "Tools/Julia/Julia.hpp"

namespace spu
{
namespace module
{
template<typename T>
void
Module_Julia::create_constant(runtime::Task& task, const T& value)
{
    if ((*this->n_clones) > 0)
    {
        std::stringstream message;
        message << "It is not possible to create a new constant when clones exist ('n_clones' = " << (*this->n_clones)
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    for (auto s : task.sockets)
        if (s->get_name() == "status")
        {
            std::stringstream message;
            message << "Creating a constant after the codelet creation is forbidden.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

    size_t tid = Module_Julia::get_task_id(task);
    (*this->jl_create_constants)[tid].push_back(
      [tid, value](Module_Julia& m)
      {
          jl_value_t* jl_constant_ptr = tools::Julia::box(value);

          // protect from garbage collector
          size_t jl_constant_id = tools::Julia::preserve(jl_constant_ptr);

          m.jl_constants_ptr[tid].push_back(jl_constant_ptr);
          m.jl_constants_id[tid].push_back(jl_constant_id);
      });
}
}
}
