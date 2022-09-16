#include "Module/Probe/Value/Probe_value.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

template <typename T>
Probe_value<T>
::Probe_value(const int size, const std::string &col_name, tools::Reporter_probe& reporter, const int n_frames)
: Probe<T>(size, col_name, reporter, n_frames)
{
	const std::string name = "Probe_value<" + col_name + ">";
	this->set_name(name);
}

template <typename T>
void Probe_value<T>
::_probe(const T *in, const size_t frame_id)
{
	this->reporter.probe(this->col_name, (void*)in, frame_id);
}

template <typename T>
std::type_index Probe_value<T>
::get_datatype() const
{
	return typeid(T);
}

// ==================================================================================== explicit template instantiation
template class aff3ct::module::Probe_value<int8_t>;
template class aff3ct::module::Probe_value<uint8_t>;
template class aff3ct::module::Probe_value<int16_t>;
template class aff3ct::module::Probe_value<uint16_t>;
template class aff3ct::module::Probe_value<int32_t>;
template class aff3ct::module::Probe_value<uint32_t>;
template class aff3ct::module::Probe_value<int64_t>;
template class aff3ct::module::Probe_value<uint64_t>;
template class aff3ct::module::Probe_value<float>;
template class aff3ct::module::Probe_value<double>;
// ==================================================================================== explicit template instantiation
