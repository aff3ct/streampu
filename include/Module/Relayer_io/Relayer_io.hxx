#include "Module/Relayer_io/Relayer_io.hpp"

namespace aff3ct
{
namespace module
{

template <typename T>
runtime::Task& Relayer_io<T>
::operator[](const rly_io::tsk t)
{
	return Module::operator[]((size_t)t);
}

template <typename T>
runtime::Socket& Relayer_io<T>
::operator[](const rly_io::sck::relay_io s)
{
	return Module::operator[]((size_t)rly_io::tsk::relay_io)[(size_t)s];
}

template <typename T>
template <class A>
void Relayer_io<T>
::relay_io(std::vector<T,A>& inout, const int frame_id, const bool managed_memory)
{
	(*this)[rly_io::sck::relay_io::inout].bind(inout);
	(*this)[rly_io::tsk::relay_io].exec(frame_id, managed_memory);
}

}
}
