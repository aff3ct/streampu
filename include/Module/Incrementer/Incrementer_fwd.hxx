#include "Module/Incrementer/Incrementer_fwd.hpp"

namespace aff3ct
{
namespace module
{

template <typename T>
runtime::Task& Incrementer_fwd<T>
::operator[](const inc_fwd::tsk t)
{
	return Module::operator[]((size_t)t);
}

template <typename T>
runtime::Socket& Incrementer_fwd<T>
::operator[](const inc_fwd::sck::increment_fwd s)
{
	return Module::operator[]((size_t)inc_fwd::tsk::increment_fwd)[(size_t)s];
}

template <typename T>
runtime::Socket& Incrementer_fwd<T>
::operator[](const std::string &tsk_sck)
{
	return Module::operator[](tsk_sck);
}

template <typename T>
template <class A>
void Incrementer_fwd<T>
::increment_fwd(std::vector<T,A>& fwd, const int frame_id, const bool managed_memory)
{
	(*this)[inc_fwd::sck::increment_fwd::fwd ].bind(fwd );
	(*this)[inc_fwd::tsk::increment_fwd].exec(frame_id, managed_memory);
}

}
}
