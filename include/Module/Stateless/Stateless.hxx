#include "Module/Stateless/Stateless.hpp"

namespace aff3ct
{
namespace module
{

template <typename T>
size_t Stateless
::create_socket_in(runtime::Task& task, const std::string &name, const size_t n_elmts)
{
	return Module::create_socket_in<T>(task, name, n_elmts);
}

template <typename T>
size_t Stateless
::create_socket_out(runtime::Task& task, const std::string &name, const size_t n_elmts)
{
	return Module::create_socket_out<T>(task, name, n_elmts);
}

// Socket FWD
template <typename T>
size_t Stateless
::create_socket_inout(runtime::Task& task, const std::string &name, const size_t n_elmts)
{
	return Module::create_socket_inout<T>(task, name, n_elmts);
}

}
}
