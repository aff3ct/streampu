/*!
 * \file
 * \brief Class module::Stateless_Julia.
 */
#ifndef STATELESS_JULIA_HPP_
#define STATELESS_JULIA_HPP_

#ifdef SPU_JULIA

#include <string>

#include "Module/Module_Julia.hpp"
#include "Module/Stateless/Stateless.hpp"
#include "Runtime/Task/Task.hpp"

namespace spu
{
namespace module
{
class Stateless_Julia final
  : public Stateless
  , public Module_Julia
{
  public:
    Stateless_Julia(const bool jl_safe = true);
    virtual ~Stateless_Julia() = default;
    virtual Stateless_Julia* clone() const;
    void deep_copy(const Stateless_Julia& m);

    runtime::Task& create_task(const std::string& name);
    runtime::Task& create_tsk(const std::string& name);

    using Module_Julia::create_cdl;
    using Module_Julia::create_codelet;
};
}
}

#endif /* SPU_JULIA */

#endif /* STATELESS_JULIA_HPP_ */
