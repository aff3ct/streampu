#include "Module/Stateless/Stateless.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

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
