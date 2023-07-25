#include <chrono>
#include <sstream>

#include "Module/Incrementer/Incrementer_fwd.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

template <typename T>
Incrementer_fwd<T>
::Incrementer_fwd(const size_t n_elmts, const size_t ns)
: Module(), n_elmts(n_elmts), ns(ns)
{
	const std::string name = "Incrementer_inputOutput";
	this->set_name(name);
	this->set_short_name(name);

	if (n_elmts == 0)
	{
		std::stringstream message;
		message << "'n_elmts' has to be greater than 0 ('n_elmts' = " << n_elmts << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	auto &p = this->create_task("increment");
	auto ps_fwd  = this->template create_socket_fwd <T>(p, "fwd", this->n_elmts);
	this->create_codelet(p, [ps_fwd](Module &m, runtime::Task &t, const size_t frame_id) -> int
	{
		auto &inc = static_cast<Incrementer_fwd&>(m);
		inc._increment_fwd(static_cast<T*>(t[ps_fwd].get_dataptr()), frame_id);
		return runtime::status_t::SUCCESS;
	});
}

template <typename T>
Incrementer_fwd<T>* Incrementer_fwd<T>
::clone() const
{
	auto m = new Incrementer_fwd(*this);
	m->deep_copy(*this);
	return m;
}

template <typename T>
size_t Incrementer_fwd<T>
::get_ns() const
{
	return this->ns;
}

template <typename T>
size_t Incrementer_fwd<T>
::get_n_elmts() const
{
	return this->n_elmts;
}

template <typename T>
void Incrementer_fwd<T>
::set_ns(const size_t ns)
{
	this->ns = ns;
}

template <typename T>
void Incrementer_fwd<T>
::increment_fwd( T *fwd, const int frame_id, const bool managed_memory)
{
	(*this)[inc_fwd::sck::increment_fwd::fwd ].bind(fwd );
	(*this)[inc_fwd::tsk::increment_fwd].exec(frame_id, managed_memory);
}

template <typename T>
void Incrementer_fwd<T>
::_increment_fwd(T *fwd, const size_t frame_id)
{
	std::chrono::time_point<std::chrono::steady_clock> t_start;
	if (this->ns)
		t_start = std::chrono::steady_clock::now();

	
	for (size_t e = 0; e < this->n_elmts; e++)
		fwd[e] += 1;

	if (this->ns)
	{
		std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;
		while ((size_t)duration.count() < this->ns) // active waiting
			duration = std::chrono::steady_clock::now() - t_start;
	}
}

// ==================================================================================== explicit template instantiation
template class aff3ct::module::Incrementer_fwd<int8_t>;
template class aff3ct::module::Incrementer_fwd<uint8_t>;
template class aff3ct::module::Incrementer_fwd<int16_t>;
template class aff3ct::module::Incrementer_fwd<uint16_t>;
template class aff3ct::module::Incrementer_fwd<int32_t>;
template class aff3ct::module::Incrementer_fwd<uint32_t>;
template class aff3ct::module::Incrementer_fwd<int64_t>;
template class aff3ct::module::Incrementer_fwd<uint64_t>;
template class aff3ct::module::Incrementer_fwd<float>;
template class aff3ct::module::Incrementer_fwd<double>;
// ==================================================================================== explicit template instantiation

