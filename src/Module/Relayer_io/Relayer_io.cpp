#include <chrono>
#include <sstream>

#include "Module/Relayer_io/Relayer_io.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

template <typename T>
Relayer_io<T>
::Relayer_io(const size_t n_elmts, const size_t ns)
: Module(), n_elmts(n_elmts), ns(ns)
{
	const std::string name = "Relayer_io";
	this->set_name(name);
	this->set_short_name(name);

	if (n_elmts == 0)
	{
		std::stringstream message;
		message << "'n_elmts' has to be greater than 0 ('n_elmts' = " << n_elmts << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	auto &p = this->create_task("relay_io");
	auto ps_inout = this->template create_socket_inout<T>(p, "inout", this->n_elmts);
	this->create_codelet(p, [ps_inout](Module &m, runtime::Task &t, const size_t frame_id) -> int
	{
		auto &rly_io = static_cast<Relayer_io&>(m);
		rly_io._relay_io(static_cast<T*>(t[ps_inout].get_dataptr()),
		              frame_id);
		return runtime::status_t::SUCCESS;
	});
}

template <typename T>
Relayer_io<T>* Relayer_io<T>
::clone() const
{
	auto m = new Relayer_io(*this);
	m->deep_copy(*this);
	return m;
}

template <typename T>
size_t Relayer_io<T>
::get_ns() const
{
	return this->ns;
}

template <typename T>
size_t Relayer_io<T>
::get_n_elmts() const
{
	return this->n_elmts;
}

template <typename T>
void Relayer_io<T>
::set_ns(const size_t ns)
{
	this->ns = ns;
}

template <typename T>
void Relayer_io<T>
::relay_io(T *inout, const int frame_id, const bool managed_memory)
{
	(*this)[rly_io::sck::relay_io::inout].bind(inout);
	(*this)[rly_io::tsk::relay_io].exec(frame_id, managed_memory);
}

template <typename T>
void Relayer_io<T>
::_relay_io(T *inout, const size_t frame_id)
{
	std::chrono::time_point<std::chrono::steady_clock> t_start;
	if (this->ns)
		t_start = std::chrono::steady_clock::now();

	//std::copy(in, in + this->n_elmts, out);

	if (this->ns)
	{
		std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;
		while ((size_t)duration.count() < this->ns) // active waiting
			duration = std::chrono::steady_clock::now() - t_start;
	}
}

// ==================================================================================== explicit template instantiation
template class aff3ct::module::Relayer_io<int8_t>;
template class aff3ct::module::Relayer_io<uint8_t>;
template class aff3ct::module::Relayer_io<int16_t>;
template class aff3ct::module::Relayer_io<uint16_t>;
template class aff3ct::module::Relayer_io<int32_t>;
template class aff3ct::module::Relayer_io<uint32_t>;
template class aff3ct::module::Relayer_io<int64_t>;
template class aff3ct::module::Relayer_io<uint64_t>;
template class aff3ct::module::Relayer_io<float>;
template class aff3ct::module::Relayer_io<double>;
// ==================================================================================== explicit template instantiation
