#include <ios>
#include <sstream>

#include "Tools/Exception/exception.hpp"
#include "Module/Probe/Stream/Probe_stream.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

Probe_stream
::Probe_stream(const std::string &col_name, tools::Reporter_probe* reporter)
: Probe<uint8_t>(0, col_name),
  occurrences(0)
{
	const std::string name = "Probe_stream<" + col_name + ">";
	this->set_name(name);
	this->set_single_wave(true);

	if (reporter != nullptr)
		this->register_reporter(reporter);
}

void Probe_stream
::register_reporter(tools::Reporter_probe* reporter)
{
	if (this->reporter != nullptr)
	{
		std::stringstream message;
		message << "It is not possible to register this probe to a new 'tools::Reporter_probe' because it is already "
		        << "registered to an other 'tools::Reporter_probe'.";
		throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
	}
	this->reporter = reporter;
	this->reporter->register_probe(this, 1, typeid(int64_t), "", 100, std::ios_base::scientific, 3);
}

void Probe_stream
::_probe(const uint8_t *in, const size_t frame_id)
{
	this->check_reporter();

	for (size_t f = 0; f < this->get_n_frames(); f++)
	{
		this->reporter->probe(this->col_name, (void*)&occurrences, frame_id);
	}
	this->occurrences++;
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
