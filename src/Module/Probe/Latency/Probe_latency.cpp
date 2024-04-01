#include <ios>
#include <sstream>

#include "Module/Probe/Latency/Probe_latency.hpp"
#include "Tools/Exception/exception.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

Probe_latency::Probe_latency(const std::string& col_name, tools::Reporter_probe* reporter)
  : Probe<uint8_t>(0, col_name)
  , t_start(std::chrono::steady_clock::now())
{
    const std::string name = "Probe_latency<" + col_name + ">";
    this->set_name(name);
    this->set_single_wave(true);

    if (reporter != nullptr) this->register_reporter(reporter);
}

void
Probe_latency::register_reporter(tools::Reporter_probe* reporter)
{
    if (this->reporter != nullptr)
    {
        std::stringstream message;
        message << "It is not possible to register this probe to a new 'tools::Reporter_probe' because it is already "
                << "registered to an other 'tools::Reporter_probe'.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    this->reporter = reporter;
    this->proxy_register_probe(1, typeid(int64_t), "(us)", 100, std::ios_base::scientific, 3);
}

void
Probe_latency::_probe(const uint8_t* in, const size_t frame_id)
{
    auto t_stop = std::chrono::steady_clock::now();
    auto time_duration = (int64_t)std::chrono::duration_cast<std::chrono::microseconds>(t_stop - this->t_start).count();
    this->t_start = t_stop;

    for (size_t f = 0; f < this->get_n_frames(); f++)
        this->proxy_probe((void*)&time_duration, frame_id);
}

void
Probe_latency::reset()
{
    this->t_start = std::chrono::steady_clock::now();
}
