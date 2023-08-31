#include "Module/Relayer/Relayer_fwd.hpp"

namespace aff3ct
{
namespace module
{

template <typename T>
runtime::Task& Relayer_fwd<T>
::operator[](const rly_fwd::tsk t)
{
	return Module::operator[]((size_t)t);
}

template <typename T>
runtime::Socket& Relayer_fwd<T>
::operator[](const rly_fwd::sck::relay_fwd s)
{
	return Module::operator[]((size_t)rly_fwd::tsk::relay_fwd)[(size_t)s];
}

template <typename T>
template <class A>
void Relayer_fwd<T>
::relay_fwd(std::vector<T,A>& fwd, const int frame_id, const bool managed_memory)
{
	(*this)[rly_fwd::sck::relay_fwd::fwd].bind(fwd);
	(*this)[rly_fwd::tsk::relay_fwd].exec(frame_id, managed_memory);
}

}
}
