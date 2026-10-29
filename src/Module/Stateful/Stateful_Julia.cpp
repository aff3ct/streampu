#ifdef SPU_JULIA

#include "Module/Stateful/Stateful_Julia.hpp"

using namespace spu;
using namespace spu::module;

Stateful_Julia::Stateful_Julia(const bool jl_safe)
  : Stateful()
  , Module_Julia(
      this,
      [](Module& m) -> Module_Julia& { return static_cast<Stateful_Julia&>(m); },
      jl_safe)
{
    const std::string name = "Stateful_Julia";
    this->set_name(name);
    this->set_short_name(name);
}

Stateful_Julia*
Stateful_Julia::clone() const
{
    auto m = new Stateful_Julia(*this);
    m->deep_copy(*this);
    return m;
}

void
Stateful_Julia::deep_copy(const Stateful_Julia& m)
{
    Stateful::deep_copy(m);
    Module_Julia::deep_copy(m, this);
}

runtime::Task&
Stateful_Julia::create_task(const std::string& name)
{
    return this->_create_task(name);
}

runtime::Task&
Stateful_Julia::create_tsk(const std::string& name)
{
    return this->_create_task(name);
}

void
Stateful_Julia::create_state(const std::string& julia_constructor,
                             const bool shared,
                             const std::string& julia_code)
{
    this->_create_state(julia_constructor, shared, julia_code);
}

void
Stateful_Julia::create_stt(const std::string& julia_constructor,
                           const bool shared,
                           const std::string& julia_code)
{
    this->_create_state(julia_constructor, shared, julia_code);
}

void
Stateful_Julia::create_state_file(const std::string& julia_constructor,
                                  const bool shared,
                                  const std::string& julia_filepath)
{
    this->_create_state_file(julia_constructor, shared, julia_filepath);
}

void
Stateful_Julia::create_stt_file(const std::string& julia_constructor,
                                const bool shared,
                                const std::string& julia_filepath)
{
    this->_create_state_file(julia_constructor, shared, julia_filepath);
}

const std::vector<jl_value_t*>&
Stateful_Julia::get_states() const
{
    return this->_get_states();
}

#endif /* SPU_JULIA */
