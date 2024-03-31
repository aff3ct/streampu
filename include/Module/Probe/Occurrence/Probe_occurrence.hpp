/*!
 * \file
 * \brief Class module::Probe_occurrence.
 */
#ifndef PROBE_OCCURRENCE_HPP_
#define PROBE_OCCURRENCE_HPP_

#include <string>
#include <vector>
#include <cstdint>
#include <typeindex>

#include "Module/Probe/Probe.hpp"

namespace aff3ct
{
namespace module
{
class Probe_occurrence : public Probe<uint8_t>
{
protected:
	int64_t occurrences;

public:
	Probe_occurrence(const std::string &col_name, tools::Reporter_probe* reporter = nullptr);

	virtual ~Probe_occurrence() = default;

	virtual void reset();

	virtual void register_reporter(tools::Reporter_probe* reporter);

	int64_t get_occurrences() const;

protected:
	virtual void _probe(const uint8_t *in, const size_t frame_id);
};
}
}

#endif /* PROBE_OCCURRENCE_HPP_ */
