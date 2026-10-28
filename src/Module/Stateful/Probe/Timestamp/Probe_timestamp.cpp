#include <chrono>
#include <ctime>
#include <ios>
#include <sstream>

#include "Module/Stateful/Probe/Timestamp/Probe_timestamp.hpp"
#include "Tools/Exception/exception.hpp"

using namespace spu;
using namespace spu::module;

Probe_timestamp::Probe_timestamp(const uint64_t mod, const std::string& col_name, tools::Reporter_probe* reporter)
  : Probe<uint8_t>(0, col_name)
  , mod(mod)
{
    const std::string name = "Probe_timestamp<" + col_name + ">";
    this->set_name(name);
    this->set_single_wave(true);

    if (reporter != nullptr) this->register_reporter(reporter);
}

Probe_timestamp::Probe_timestamp(const std::string& col_name, tools::Reporter_probe* reporter)
  : Probe_timestamp(0, col_name, reporter)
{
}

void
Probe_timestamp::register_reporter(tools::Reporter_probe* reporter)
{
    if (this->reporter != nullptr)
    {
        std::stringstream message;
        message << "It is not possible to register this probe to a new 'tools::Reporter_probe' because it is already "
                << "registered to an other 'tools::Reporter_probe'.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    this->reporter = reporter;
    this->proxy_register_probe(1, typeid(uint64_t), "(us)", 100, std::ios_base::scientific, 2);
}

void
Probe_timestamp::_probe(const uint8_t* /*in*/, const size_t frame_id)
{
    std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(
      // std::chrono::system_clock::now().time_since_epoch()
      std::chrono::steady_clock::now().time_since_epoch());

    uint64_t unix_timestamp_count = mod ? (uint64_t)us.count() % mod : (uint64_t)us.count();

    for (size_t f = 0; f < this->get_n_frames(); f++)
        this->proxy_probe((void*)&unix_timestamp_count, frame_id);
}
