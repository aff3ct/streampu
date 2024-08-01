#include "Module/Stateful/Switcher/Switcher.hpp"

using namespace spu;
using namespace spu::module;

Switcher*
Switcher::clone() const
{
    auto m = new Switcher(*this);
    m->deep_copy(*this);
    return m;
}

void
Switcher::reset()
{
    this->path = this->get_n_data_sockets() - 1;
}
