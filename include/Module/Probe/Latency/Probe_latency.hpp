/*!
 * \file
 * \brief Class module::Probe_latency.
 */
#ifndef PROBE_LATENCY_HPP_
#define PROBE_LATENCY_HPP_

#include <chrono>
#include <string>
#include <typeindex>
#include <vector>

#include "Module/Probe/Probe.hpp"

namespace spu
{
namespace module
{
class Probe_latency : public Probe<uint8_t>
{
  protected:
    std::chrono::time_point<std::chrono::steady_clock> t_start;

  public:
    Probe_latency(const std::string& col_name, tools::Reporter_probe* reporter = nullptr);

    virtual ~Probe_latency() = default;

    virtual void reset();

    virtual void register_reporter(tools::Reporter_probe* reporter);

  protected:
    virtual void _probe(const uint8_t* in, const size_t frame_id);
};
}
}

#endif /* PROBE_LATENCY_HPP_ */
