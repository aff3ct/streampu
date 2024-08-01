/*!
 * \file
 * \brief Class module::Probe_stream.
 */
#ifndef PROBE_STREAM_HPP_
#define PROBE_STREAM_HPP_

#include <cstdint>
#include <string>
#include <typeindex>
#include <vector>

#include "Module/Stateful/Probe/Probe.hpp"

namespace spu
{
namespace module
{
class Probe_stream : public Probe<uint8_t>
{
  protected:
    int64_t occurrences;

  public:
    Probe_stream(const std::string& col_name, tools::Reporter_probe* reporter = nullptr);

    virtual ~Probe_stream() = default;

    virtual void reset();

    virtual void register_reporter(tools::Reporter_probe* reporter);

    int64_t get_occurrences() const;

  protected:
    virtual void _probe(const uint8_t* in, const size_t frame_id);
};
}
}

#endif /* PROBE_STREAM_HPP_ */
