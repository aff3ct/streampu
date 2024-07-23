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
