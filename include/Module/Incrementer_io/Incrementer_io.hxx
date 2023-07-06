#include "Module/Incrementer_io/Incrementer_io.hpp"

namespace aff3ct
{
namespace module
{

template <typename T>
runtime::Task& Incrementer_io<T>
::operator[](const inc_io::tsk t)
{
	return Module::operator[]((size_t)t);
}

template <typename T>
runtime::Socket& Incrementer_io<T>
::operator[](const inc_io::sck::increment_io s)
{
	return Module::operator[]((size_t)inc_io::tsk::increment_io)[(size_t)s];
}

template <typename T>
template <class A>
void Incrementer_io<T>
::increment_io(std::vector<T,A>& inout, const int frame_id, const bool managed_memory)
{
	(*this)[inc_io::sck::increment_io::inout ].bind(inout );
	(*this)[inc_io::tsk::increment_io].exec(frame_id, managed_memory);
}

}
}


