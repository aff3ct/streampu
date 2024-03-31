/*!
 * \file
 * \brief Class module::Probe_timestamp.
 */
#ifndef PROBE_TIMESTAMP_HPP_
#define PROBE_TIMESTAMP_HPP_

#include <string>
#include <typeindex>
#include <typeindex>

#include "Module/Probe/Probe.hpp"

namespace aff3ct
{
namespace module
{
class Probe_timestamp : public Probe<uint8_t>
{
protected:
	const uint64_t mod;

public:
	Probe_timestamp(const uint64_t mod, const std::string &col_name, tools::Reporter_probe* reporter = nullptr);
	Probe_timestamp(const std::string &col_name, tools::Reporter_probe* reporter = nullptr);

	virtual ~Probe_timestamp() = default;

	virtual void register_reporter(tools::Reporter_probe* reporter);

protected:
	virtual void _probe(const uint8_t *in, const size_t frame_id);
};
}
}

#endif /* PROBE_TIMESTAMP_HPP_ */
