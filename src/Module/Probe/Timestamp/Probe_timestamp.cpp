#include <ctime>
#include <chrono>

#include "Module/Probe/Timestamp/Probe_timestamp.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

Probe_timestamp
::Probe_timestamp(const std::string &col_name, const uint64_t mod, tools::Reporter_probe& reporter)
: Probe<uint8_t>(0, col_name, reporter), mod(mod)
{
	const std::string name = "Probe_timestamp<" + col_name + ">";
	this->set_name(name);
	this->set_single_wave(true);
}

Probe_timestamp
::Probe_timestamp(const std::string &col_name, tools::Reporter_probe& reporter)
: Probe_timestamp(col_name, 0, reporter)
{
}

void Probe_timestamp
::_probe(const uint8_t *in, const size_t frame_id)
{
	std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(
	    // std::chrono::system_clock::now().time_since_epoch()
	    std::chrono::steady_clock::now().time_since_epoch()
	);

	uint64_t unix_timestamp_count = mod ? (uint64_t)us.count() % mod : (uint64_t)us.count();

	for (size_t f = 0; f < this->get_n_frames(); f++)
		this->reporter.probe(this->col_name, (void*)&unix_timestamp_count, frame_id);
}


std::type_index Probe_timestamp
::get_datatype() const
{
	return typeid(uint64_t);
}
