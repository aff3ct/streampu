/*!
 * \file
 * \brief Class module::Probe_stream.
 */
#ifndef PROBE_STREAM_HPP_
#define PROBE_STREAM_HPP_

#include <string>
#include <vector>
#include <cstdint>
#include <typeindex>

#include "Module/Probe/Probe.hpp"

namespace aff3ct
{
namespace module
{
class Probe_stream : public Probe<uint8_t>
{
	friend tools::Reporter_probe;

protected:
	int64_t occurrences;
	Probe_stream(const std::string &col_name, tools::Reporter_probe& reporter);

public:
	virtual ~Probe_stream() = default;

	virtual std::type_index get_datatype() const;

	virtual void reset();

	int64_t get_occurrences() const;

protected:
	virtual void _probe(const uint8_t *in, const size_t frame_id);
};
}
}

#endif /* PROBE_STREAM_HPP_ */
