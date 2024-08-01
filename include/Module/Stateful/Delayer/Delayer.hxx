#include "Module/Stateful/Delayer/Delayer.hpp"

namespace spu
{
namespace module
{

template<typename D>
runtime::Task&
Delayer<D>::operator[](const dly::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename D>
runtime::Socket&
Delayer<D>::operator[](const dly::sck::memorize s)
{
    return Module::operator[]((size_t)dly::tsk::memorize)[(size_t)s];
}

template<typename D>
runtime::Socket&
Delayer<D>::operator[](const dly::sck::produce s)
{
    return Module::operator[]((size_t)dly::tsk::produce)[(size_t)s];
}

template<typename D>
runtime::Socket&
Delayer<D>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

}
}
