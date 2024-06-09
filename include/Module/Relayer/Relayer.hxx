#include "Module/Relayer/Relayer.hpp"

namespace spu
{
namespace module
{

template<typename T>
runtime::Task&
Relayer<T>::operator[](const rly::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Relayer<T>::operator[](const rly::sck::relay s)
{
    return Module::operator[]((size_t)rly::tsk::relay)[(size_t)s];
}

template<typename T>
runtime::Socket&
Relayer<T>::operator[](const rly::sck::relayf s)
{
    return Module::operator[]((size_t)rly::tsk::relayf)[(size_t)s];
}

template<typename T>
runtime::Socket&
Relayer<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A>
void
Relayer<T>::relay(const std::vector<T, A>& in, std::vector<T, A>& out, const int frame_id, const bool managed_memory)
{
    (*this)[rly::sck::relay::in].bind(in);
    (*this)[rly::sck::relay::out].bind(out);
    (*this)[rly::tsk::relay].exec(frame_id, managed_memory);
}

}
}
