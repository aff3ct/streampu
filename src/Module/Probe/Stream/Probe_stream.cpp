#include "Module/Probe/Stream/Probe_stream.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

Probe_stream
::Probe_stream(const std::string &col_name, tools::Reporter_probe& reporter)
: Probe<uint8_t>(0, col_name, reporter),
  occurrences(0)
{
	const std::string name = "Probe_stream<" + col_name + ">";
	this->set_name(name);
	this->set_single_wave(true);
}

void Probe_stream
::_probe(const uint8_t *in, const size_t frame_id)
{
	for (size_t f = 0; f < this->get_n_frames(); f++)
	{
		this->reporter.probe(this->col_name, (void*)&occurrences, frame_id);
	}
	this->occurrences++;
}

std::type_index Probe_stream
::get_datatype() const
{
	return typeid(int64_t);
}

void Probe_stream
::reset()
{
	this->occurrences = 0;
}

int64_t Probe_stream
::get_occurrences() const
{
	return this->occurrences;
}
