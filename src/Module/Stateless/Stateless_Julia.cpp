#ifdef SPU_JULIA
#ifndef SPU_JLUNA

#include "Module/Stateless/Stateless_Julia.hpp"

using namespace spu;
using namespace spu::module;

Stateless_Julia::Stateless_Julia(const bool jl_safe)
  : Stateless()
  , Module_Julia(
      this,
      [](Module& m) -> Module_Julia& { return static_cast<Stateless_Julia&>(m); },
      jl_safe)
{
    const std::string name = "Stateless_Julia";
    this->set_name(name);
    this->set_short_name(name);
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
    Stateless::deep_copy(m);
    Module_Julia::deep_copy(m, this);
}

runtime::Task&
Stateless_Julia::create_task(const std::string& name)
{
    return this->_create_task(name);
}

runtime::Task&
Stateless_Julia::create_tsk(const std::string& name)
{
    return this->_create_task(name);
}

#endif /* SPU_JLUNA */
#endif /* SPU_JULIA */
