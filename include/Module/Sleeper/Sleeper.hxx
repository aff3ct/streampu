#include "Module/Sleeper/Sleeper.hpp"

namespace spu
{
namespace module
{

runtime::Task&
Sleeper::operator[](const slp::tsk t)
{
    return Module::operator[]((size_t)t);
}

runtime::Socket&
Sleeper::operator[](const slp::sck::sleep s)
{
    return Module::operator[]((size_t)slp::tsk::sleep)[(size_t)s];
}

runtime::Socket&
Sleeper::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

}
}
