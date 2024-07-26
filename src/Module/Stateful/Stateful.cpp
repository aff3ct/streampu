#include "Module/Stateful/Stateful.hpp"

using namespace spu;
using namespace spu::module;

Stateful::Stateful()
  : Module()
{
}

void
Stateful::deep_copy(const Stateful& m)
{
    Module::deep_copy(m);
}