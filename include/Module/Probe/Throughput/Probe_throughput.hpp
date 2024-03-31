/*!
 * \file
 * \brief Class module::Probe_throughput.
 */
#ifndef PROBE_THROUGHPUT_HPP_
#define PROBE_THROUGHPUT_HPP_

#include <string>
#include <vector>
#include <chrono>
#include <typeindex>

#include "Module/Probe/Probe.hpp"

namespace aff3ct
{
namespace module
{
class Probe_throughput : public Probe<uint8_t>
{
protected:
	std::chrono::time_point<std::chrono::steady_clock> t_start;
	size_t data_size;
	double thr;
	double factor;

public:
	Probe_throughput(const size_t data_size, const std::string &col_name, const double factor,
	                 tools::Reporter_probe* reporter = nullptr);
	Probe_throughput(const size_t data_size, const std::string &col_name, tools::Reporter_probe* reporter = nullptr);
	Probe_throughput(const std::string &col_name, tools::Reporter_probe* reporter = nullptr);

	virtual ~Probe_throughput() = default;

	virtual void reset();

	virtual void register_reporter(tools::Reporter_probe* reporter);

protected:
	virtual void _probe(const uint8_t *in, const size_t frame_id);
};
}
}

#endif /* PROBE_THROUGHPUT_HPP_ */
