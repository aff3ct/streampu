#include <chrono>
#include <sstream>
#include <string>
#include <thread>

#include "Module/Stateful/Initializer/Initializer.hpp"

namespace spu
{
namespace module
{

template<typename T>
runtime::Task&
Initializer<T>::operator[](const ini::tsk t)
{
    return Module::operator[]((size_t)t);
}

template<typename T>
runtime::Socket&
Initializer<T>::operator[](const ini::sck::initialize s)
{
    return Module::operator[]((size_t)ini::tsk::initialize)[(size_t)s];
}

template<typename T>
runtime::Socket&
Initializer<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
template<class A>
void
Initializer<T>::initialize(std::vector<T, A>& out, const int frame_id, const bool managed_memory)
{
    (*this)[ini::sck::initialize::out].bind(out);
    (*this)[ini::tsk::initialize].exec(frame_id, managed_memory);
}

}
}
