#include <string>
#include <sstream>

#include "Tools/Exception/exception.hpp"
#include "Module/Source/Source.hpp"

namespace aff3ct
{
namespace module
{

template <typename B>
runtime::Task& Source<B>
::operator[](const src::tsk t)
{
	return Module::operator[]((size_t)t);
}

template <typename B>
runtime::Socket& Source<B>
::operator[](const src::sck::generate s)
{
	return Module::operator[]((size_t)src::tsk::generate)[(size_t)s];
}

template <typename B>
runtime::Socket& Source<B>
::operator[](const std::string &tsk_sck)
{
	return Module::operator[](tsk_sck);
}

template <typename B>
Source<B>
::Source(const int max_data_size)
: Module(), max_data_size(max_data_size)
{
	const std::string name = "Source";
	this->set_name(name);
	this->set_short_name(name);

	if (max_data_size <= 0)
	{
		std::stringstream message;
		message << "'max_data_size' has to be greater than 0 ('max_data_size' = " << max_data_size << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	auto &p = this->create_task("generate");
	auto ps_out_data = this->template create_socket_out<B>(p, "out_data", this->max_data_size);
	auto ps_out_count = this->template create_socket_out<uint32_t>(p, "out_count", 1);
	this->create_codelet(p, [ps_out_data, ps_out_count](Module &m, runtime::Task &t, const size_t frame_id) -> int
	{
		auto &src = static_cast<Source<B>&>(m);

		src._generate(t[ps_out_data].template get_dataptr<B>(),
			          t[ps_out_count].template get_dataptr<uint32_t>(),
		              frame_id);

		return runtime::status_t::SUCCESS;
	});
}

template <typename B>
Source<B>* Source<B>
::clone() const
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template <typename B>
int Source<B>
::get_max_data_size() const
{
	return max_data_size;
}

template <typename B>
template <class A>
void Source<B>
::generate(std::vector<B,A>& out_data, const int frame_id, const bool managed_memory)
{
	(*this)[src::sck::generate::out_data].bind(out_data);
	(*this)[src::tsk::generate].exec(frame_id, managed_memory);
}

template <typename B>
void Source<B>
::generate(B *out_data, const int frame_id, const bool managed_memory)
{
	(*this)[src::sck::generate::out_data].bind(out_data);
	(*this)[src::tsk::generate].exec(frame_id, managed_memory);
}

template <typename B>
template <class A>
void Source<B>
::generate(std::vector<B,A>& out_data, std::vector<uint32_t>& out_count, const int frame_id, const bool managed_memory)
{
	(*this)[src::sck::generate::out_data].bind(out_data);
	(*this)[src::sck::generate::out_count].bind(out_count);
	(*this)[src::tsk::generate].exec(frame_id, managed_memory);
}

template <typename B>
void Source<B>
::generate(B *out_data, uint32_t* out_count, const int frame_id, const bool managed_memory)
{
	(*this)[src::sck::generate::out_data].bind(out_data);
	(*this)[src::sck::generate::out_count].bind(out_count);
	(*this)[src::tsk::generate].exec(frame_id, managed_memory);
}

template <typename B>
void Source<B>
::_generate(B *out_data, const size_t frame_id)
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template <typename B>
void Source<B>
::_generate(B *out_data, uint32_t *out_count, const size_t frame_id)
{
	this->_generate(out_data, frame_id);
	*out_count = this->max_data_size;
}

template <typename B>
void Source<B>
::set_seed(const int seed)
{
	// do nothing in the general case, this method has to be overrided
}

template <typename B>
bool Source<B>
::is_done() const
{
	return false;
}

template <typename B>
void Source<B>
::reset()
{
	// do nothing in the general case, this method has to be overrided
}

}
}
