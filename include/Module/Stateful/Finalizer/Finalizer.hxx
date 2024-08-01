#include "Module/Stateful/Finalizer/Finalizer.hpp"

namespace spu
{
namespace module
{

template<typename T>
runtime::Task&
Finalizer<T>::operator[](const fin::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Finalizer<T>::operator[](const fin::sck::finalize s)
{
    return Module::operator[]((size_t)fin::tsk::finalize)[(size_t)s];
}

template<typename T>
runtime::Socket&
Finalizer<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A>
void
Finalizer<T>::finalize(const std::vector<T, A>& in, const int frame_id, const bool managed_memory)
{
    (*this)[fin::sck::finalize::in].bind(in);
    (*this)[fin::tsk::finalize].exec(frame_id, managed_memory);
}

}
}
