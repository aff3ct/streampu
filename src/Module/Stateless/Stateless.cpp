#include "Module/Stateless/Stateless.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

Stateless
::Stateless()
: Module()
{
}

Stateless* Stateless
::clone() const
{
	auto m = new Stateless(*this);
	m->deep_copy(*this);
	return m;
}

void Stateless
::set_name(const std::string &name)
{
	Module::set_name(name);
}

void Stateless
::set_short_name(const std::string &short_name)
{
	Module::set_short_name(short_name);
}

runtime::Task& Stateless
::create_task(const std::string &name, const int id)
{
	return Module::create_task(name, id);
}

size_t Stateless
::create_socket_in(runtime::Task& task, const std::string &name, const size_t n_elmts, const std::type_index& datatype)
{
	return Module::create_socket_in(task, name, n_elmts, datatype);
}

size_t Stateless
::create_socket_out(runtime::Task& task, const std::string &name, const size_t n_elmts, const std::type_index& datatype)
{
	return Module::create_socket_out(task, name, n_elmts, datatype);
}

// Les sockets FWD
size_t Stateless
::create_socket_inout(runtime::Task& task, const std::string &name, const size_t n_elmts, const std::type_index& datatype)
{
	return Module::create_socket_inout(task, name, n_elmts, datatype);
}


void Stateless
::create_codelet(runtime::Task& task, std::function<int(Module &m, runtime::Task &t, const size_t frame_id)> codelet)
{
	Module::create_codelet(task, codelet);
}

void Stateless
::register_timer(runtime::Task& task, const std::string &key)
{
	Module::register_timer(task, key);
}

void Stateless
::set_n_frames_per_wave(const size_t n_frames_per_wave)
{
	Module::set_n_frames_per_wave(n_frames_per_wave);
}

void Stateless
::set_single_wave(const bool enable_single_wave)
{
	Module::set_single_wave(enable_single_wave);
}