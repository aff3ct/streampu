#include "Module/Probe/Occurrence/Probe_occurrence.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

Probe_occurrence
::Probe_occurrence(const std::string &col_name, tools::Reporter_probe& reporter, const int n_frames)
: Probe<uint8_t>(0, col_name, reporter, n_frames),
  occurrences(0)
{
	const std::string name = "Probe_occurrence<" + col_name + ">";
	this->set_name(name);
	this->set_single_wave(true);
}

void Probe_occurrence
::_probe(const uint8_t *in, const size_t frame_id)
{
	for (size_t f = 0; f < this->get_n_frames(); f++)
	{
		this->reporter.probe(this->col_name, (void*)&occurrences, frame_id);
		this->occurrences++;
	}
}

std::type_index Probe_occurrence
::get_datatype() const
{
	return typeid(int64_t);
}

void Probe_occurrence
::reset()
{
	this->occurrences = 0;
}

int64_t Probe_occurrence
::get_occurrences() const
{
	return this->occurrences;
}
