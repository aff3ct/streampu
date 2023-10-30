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
::create_sck_in(runtime::Task& task, const std::string &name, const size_t n_elmts)
{
	return Module::create_sck_in<T>(task, name, n_elmts);
}

template <typename T>
size_t Stateless
::create_socket_out(runtime::Task& task, const std::string &name, const size_t n_elmts)
{
	return Module::create_socket_out<T>(task, name, n_elmts);
}

template <typename T>
size_t Stateless
::create_sck_out(runtime::Task& task, const std::string &name, const size_t n_elmts)
{
	return Module::create_sck_out<T>(task, name, n_elmts);
}

template <typename T>
size_t Stateless
::create_socket_fwd(runtime::Task& task, const std::string &name, const size_t n_elmts)
{
	return Module::create_socket_fwd<T>(task, name, n_elmts);
}

template <typename T>
size_t Stateless
::create_sck_fwd(runtime::Task& task, const std::string &name, const size_t n_elmts)
{
	return Module::create_sck_fwd<T>(task, name, n_elmts);
}

template <typename T>
size_t Stateless
::create_2d_socket_in(runtime::Task& task, const std::string &name, const size_t n_rows, const size_t n_cols)
{
	return Module::create_2d_socket_in<T>(task, name, n_rows, n_cols);
}

template <typename T>
size_t Stateless
::create_2d_sck_in(runtime::Task& task, const std::string &name, const size_t n_rows, const size_t n_cols)
{
	return Module::create_2d_sck_in<T>(task, name, n_rows, n_cols);
}

template <typename T>
size_t Stateless
::create_2d_socket_out(runtime::Task& task, const std::string &name, const size_t n_rows, const size_t n_cols)
{
	return Module::create_2d_socket_out<T>(task, name, n_rows, n_cols);
}

template <typename T>
size_t Stateless
::create_2d_sck_out(runtime::Task& task, const std::string &name, const size_t n_rows, const size_t n_cols)
{
	return Module::create_2d_sck_out<T>(task, name, n_rows, n_cols);
}

template <typename T>
size_t Stateless
::create_2d_socket_fwd(runtime::Task& task, const std::string &name, const size_t n_rows, const size_t n_cols)
{
	return Module::create_2d_socket_fwd<T>(task, name, n_rows, n_cols);
}

template <typename T>
size_t Stateless
::create_2d_sck_fwd(runtime::Task& task, const std::string &name, const size_t n_rows, const size_t n_cols)
{
	return Module::create_2d_sck_fwd<T>(task, name, n_rows, n_cols);
}

}
}
