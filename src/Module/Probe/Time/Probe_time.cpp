#include "Module/Probe/Time/Probe_time.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

Probe_time
::Probe_time(const std::string &col_name, tools::Reporter_probe& reporter, const int n_frames)
: Probe<uint8_t>(0, col_name, reporter, n_frames),
  t_start(std::chrono::steady_clock::now())
{
	const std::string name = "Probe_time<" + col_name + ">";
	this->set_name(name);
	this->set_single_wave(true);
}

void Probe_time
::_probe(const uint8_t *in, const size_t frame_id)
{
	auto t_stop = std::chrono::steady_clock::now();
	auto time_duration = (double)std::chrono::duration_cast<std::chrono::microseconds>(t_stop - this->t_start).count();
	auto time_duration_sec = time_duration * 1e-6;

	for (size_t f = 0; f < this->get_n_frames(); f++)
		this->reporter.probe(this->col_name, (void*)&time_duration_sec, frame_id);
}

std::type_index Probe_time
::get_datatype() const
{
	return typeid(double);
}

void Probe_time
::reset()
{
	this->t_start = std::chrono::steady_clock::now();
}
