#include <chrono>
#include <sstream>
#include <string>
#include <thread>

#include "Module/Iterator/Iterator.hpp"

namespace spu
{
namespace module
{

runtime::Task&
Iterator::operator[](const ite::tsk t)
{
    return Module::operator[]((size_t)t);
}

runtime::Socket&
Iterator::operator[](const ite::sck::iterate s)
{
    return Module::operator[]((size_t)ite::tsk::iterate)[(size_t)s];
}

runtime::Socket&
Iterator::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<class A>
void
Iterator::iterate(std::vector<int8_t, A>& out, const int frame_id, const bool managed_memory)
{
    (*this)[ite::sck::iterate::out].bind(out);
    (*this)[ite::tsk::iterate].exec(frame_id, managed_memory);
}

}
}
