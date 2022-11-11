#include "Module/Probe/Throughput/Probe_throughput.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

Probe_throughput
::Probe_throughput(const size_t data_size, const std::string &col_name, const double factor,
                   tools::Reporter_probe& reporter, const int n_frames)
: Probe<uint8_t>(0, col_name, reporter, n_frames),
  t_start(std::chrono::steady_clock::now()),
  data_size(data_size),
  thr(0.),
  factor(factor)
{
	const std::string name = "Probe_throughput<" + col_name + ">";
	this->set_name(name);
	this->set_single_wave(true);
}

Probe_throughput
::Probe_throughput(const size_t data_size, const std::string &col_name, tools::Reporter_probe& reporter,
                   const int n_frames)
: Probe_throughput(data_size, col_name, 1024. * 1024., reporter, n_frames)
{
}

void Probe_throughput
::_probe(const uint8_t *in, const size_t frame_id)
{
	auto t_stop = std::chrono::steady_clock::now();
	auto time_duration = (double)std::chrono::duration_cast<std::chrono::microseconds>(t_stop - this->t_start).count();
	this->thr = ((double)(this->data_size * this->n_frames) / (this->factor)) / (time_duration * 1e-6);
	this->t_start = t_stop;

	for (size_t f = 0; f < this->get_n_frames(); f++)
		this->reporter.probe(this->col_name, (void*)&thr, frame_id);
}

std::type_index Probe_throughput
::get_datatype() const
{
	return typeid(double);
}

void Probe_throughput
::reset()
{
	this->t_start = std::chrono::steady_clock::now();
	this->thr = 0.;
}
