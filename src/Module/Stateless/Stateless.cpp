#include "Module/Stateless/Stateless.hpp"

using namespace spu;
using namespace spu::module;

Stateless::Stateless()
  : Module()
{
}

Stateless*
Stateless::clone() const
{
    auto m = new Stateless(*this);
    m->deep_copy(*this);
    return m;
}

runtime::Task&
Stateless::create_task(const std::string& name, const int id)
{
    runtime::Task& t = Module::create_task(name, id);
    t.set_stateful(false);
    return t;
}

runtime::Task&
Stateless::create_tsk(const std::string& name, const int id)
{
    runtime::Task& t = Module::create_tsk(name, id);
    t.set_stateful(false);
    return t;
}
