#include <string>

#include "Module/Sink/NO/Sink_NO.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

template <typename B>
Sink_NO<B>
::Sink_NO(const int K)
: Sink<B>(K)
{
	const std::string name = "Sink_NO";
	this->set_name(name);
}

template <typename B>
Sink_NO<B>* Sink_NO<B>
::clone() const
{
	auto m = new Sink_NO(*this);
	m->deep_copy(*this);
	return m;
}

template <typename B>
void Sink_NO<B>
::_send_k(const B *V, const uint32_t* real_K, const size_t frame_id)
{
	// do nothing, best module ever :-D
}

// ==================================================================================== explicit template instantiation
template class aff3ct::module::Sink_NO<int8_t>;
template class aff3ct::module::Sink_NO<uint8_t>;
template class aff3ct::module::Sink_NO<int16_t>;
template class aff3ct::module::Sink_NO<uint16_t>;
template class aff3ct::module::Sink_NO<int32_t>;
template class aff3ct::module::Sink_NO<uint32_t>;
template class aff3ct::module::Sink_NO<int64_t>;
template class aff3ct::module::Sink_NO<uint64_t>;
template class aff3ct::module::Sink_NO<float>;
template class aff3ct::module::Sink_NO<double>;
// ==================================================================================== explicit template instantiation
