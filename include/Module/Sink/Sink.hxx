#include <sstream>
#include <string>

#include "Tools/Exception/exception.hpp"
#include "Module/Sink/Sink.hpp"

namespace aff3ct
{
namespace module
{

template <typename B>
runtime::Task& Sink<B>
::operator[](const snk::tsk t)
{
	return Module::operator[]((size_t)t);
}

template <typename B>
runtime::Socket& Sink<B>
::operator[](const snk::sck::send s)
{
	return Module::operator[]((size_t)snk::tsk::send)[(size_t)s];
}

template <typename B>
runtime::Socket& Sink<B>
::operator[](const snk::sck::send_count s)
{
	return Module::operator[]((size_t)snk::tsk::send_count)[(size_t)s];
}

template <typename B>
runtime::Socket& Sink<B>
::operator[](const std::string &tsk_sck)
{
	return Module::operator[](tsk_sck);
}

template <typename B>
Sink<B>
::Sink(const int max_data_size)
: Module(), max_data_size(max_data_size)
{
	const std::string name = "Sink";
	this->set_name(name);
	this->set_short_name(name);

	if (max_data_size <= 0)
	{
		std::stringstream message;
		message << "'max_data_size' has to be greater than 0 ('max_data_size' = " << max_data_size << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	auto &p1 = this->create_task("send");
	auto p1s_in_data = this->template create_socket_in<B>(p1, "in_data", max_data_size);
	this->create_codelet(p1, [p1s_in_data](Module& m, runtime::Task& t, const size_t frame_id) -> int
	{
		auto &snk = static_cast<Sink<B>&>(m);

		snk._send(t[p1s_in_data].template get_dataptr<const B>(), frame_id);

		return runtime::status_t::SUCCESS;
	});

	auto &p2 = this->create_task("send_count");
	auto p2s_in_data = this->template create_socket_in<B>(p2, "in_data", max_data_size);
	auto p2s_in_count = this->template create_socket_in<uint32_t>(p2, "in_count", 1);
	this->create_codelet(p2, [p2s_in_data, p2s_in_count](Module& m, runtime::Task& t, const size_t frame_id) -> int
	{
		auto &snk = static_cast<Sink<B>&>(m);

		snk._send_count(t[p2s_in_data].template get_dataptr<const B>(),
			            t[p2s_in_count].template get_dataptr<const uint32_t>(),
		                frame_id);

		return runtime::status_t::SUCCESS;
	});
}

template <typename B>
Sink<B>* Sink<B>
::clone() const
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template <typename B>
template <class A>
void Sink<B>
::send(const std::vector<B,A>& in_data, const int frame_id, const bool managed_memory)
{
	(*this)[snk::sck::send::in_data].bind(in_data);
	(*this)[snk::tsk::send].exec(frame_id, managed_memory);
}

template <typename B>
void Sink<B>
::send(const B *in_data, const int frame_id, const bool managed_memory)
{
	(*this)[snk::sck::send::in_data].bind(in_data);
	(*this)[snk::tsk::send].exec(frame_id, managed_memory);
}

template <typename B>
template <class A>
void Sink<B>
::send_count(const std::vector<B,A>& in_data, const std::vector<uint32_t>& in_count, const int frame_id, const bool managed_memory)
{
	(*this)[snk::sck::send_count::in_data].bind(in_data);
	(*this)[snk::sck::send_count::in_count].bind(in_count);
	(*this)[snk::tsk::send_count].exec(frame_id, managed_memory);
}

template <typename B>
void Sink<B>
::send_count(const B *in_data, uint32_t* in_count, const int frame_id, const bool managed_memory)
{
	(*this)[snk::sck::send_count::in_data].bind(in_data);
	(*this)[snk::sck::send_count::in_count].bind(in_count);
	(*this)[snk::tsk::send_count].exec(frame_id, managed_memory);
}

template <typename B>
void Sink<B>
::_send(const B *in_data, const size_t frame_id)
{
	const uint32_t in_count = (uint32_t)this->max_data_size;
	this->_send_count(in_data, &in_count, frame_id);
}

template <typename B>
void Sink<B>
::_send_count(const B *in_data, const uint32_t* in_count, const size_t frame_id)
{
	throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template <typename B>
void Sink<B>
::reset()
{
	// do nothing in the general case, this method has to be overrided
}

}
}
