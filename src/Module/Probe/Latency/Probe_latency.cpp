#include "Module/Probe/Latency/Probe_latency.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

Probe_latency
::Probe_latency(const std::string &col_name, tools::Reporter_probe& reporter, const int n_frames)
: Probe<uint8_t>(0, col_name, reporter, n_frames),
  t_start(std::chrono::steady_clock::now())
{
	const std::string name = "Probe_latency<" + col_name + ">";
	this->set_name(name);
	this->set_single_wave(true);
}

void Probe_latency
::_probe(const uint8_t *in, const size_t frame_id)
{
	auto t_stop = std::chrono::steady_clock::now();
	auto time_duration = (int64_t)std::chrono::duration_cast<std::chrono::microseconds>(t_stop - this->t_start).count();
	this->t_start = t_stop;

	for (size_t f = 0; f < this->get_n_frames(); f++)
		this->reporter.probe(this->col_name, (void*)&time_duration, frame_id);
}

std::type_index Probe_latency
::get_datatype() const
{
	return typeid(int64_t);
}

void Probe_latency
::reset()
{
	this->t_start = std::chrono::steady_clock::now();
}
