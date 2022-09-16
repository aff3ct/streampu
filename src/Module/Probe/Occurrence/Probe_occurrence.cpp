#include "Module/Probe/Occurrence/Probe_occurrence.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

template <typename T>
Probe_occurrence<T>
::Probe_occurrence(const int size, const std::string &col_name, tools::Reporter_probe& reporter, const int n_frames)
: Probe<T>(size, col_name, reporter, n_frames),
  occurrences(0)
{
	const std::string name = "Probe_occurrence<" + col_name + ">";
	this->set_name(name);
	this->set_single_wave(true);
}

template <typename T>
void Probe_occurrence<T>
::_probe(const T *in, const size_t frame_id)
{
	for (size_t f = 0; f < this->get_n_frames(); f++)
	{
		this->reporter.probe(this->col_name, (void*)&occurrences, frame_id);
		this->occurrences++;
	}
}

template <typename T>
std::type_index Probe_occurrence<T>
::get_datatype() const
{
	return typeid(int64_t);
}

template <typename T>
void Probe_occurrence<T>
::reset()
{
	this->occurrences = 0;
}

template <typename T>
int64_t Probe_occurrence<T>
::get_occurrences() const
{
	return this->occurrences;
}

// ==================================================================================== explicit template instantiation
template class aff3ct::module::Probe_occurrence<int8_t>;
template class aff3ct::module::Probe_occurrence<uint8_t>;
template class aff3ct::module::Probe_occurrence<int16_t>;
template class aff3ct::module::Probe_occurrence<uint16_t>;
template class aff3ct::module::Probe_occurrence<int32_t>;
template class aff3ct::module::Probe_occurrence<uint32_t>;
template class aff3ct::module::Probe_occurrence<int64_t>;
template class aff3ct::module::Probe_occurrence<uint64_t>;
template class aff3ct::module::Probe_occurrence<float>;
template class aff3ct::module::Probe_occurrence<double>;
// ==================================================================================== explicit template instantiation
