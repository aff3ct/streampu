#include <ios>
#include <sstream>

#include "Module/Probe/Time/Probe_time.hpp"
#include "Tools/Exception/exception.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

Probe_time::Probe_time(const std::string& col_name, tools::Reporter_probe* reporter)
  : Probe<uint8_t>(0, col_name)
  , t_start(std::chrono::steady_clock::now())
{
    const std::string name = "Probe_time<" + col_name + ">";
    this->set_name(name);
    this->set_single_wave(true);

    if (reporter != nullptr) this->register_reporter(reporter);
}

void
Probe_time::register_reporter(tools::Reporter_probe* reporter)
{
    if (this->reporter != nullptr)
    {
        std::stringstream message;
        message << "It is not possible to register this probe to a new 'tools::Reporter_probe' because it is already "
                << "registered to an other 'tools::Reporter_probe'.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    this->reporter = reporter;
    this->proxy_register_probe(1, typeid(double), "(sec)", 100, std::ios_base::dec | std::ios_base::fixed, 2);
}

void
Probe_time::_probe(const uint8_t* in, const size_t frame_id)
{
    auto t_stop = std::chrono::steady_clock::now();
    auto time_duration = (double)std::chrono::duration_cast<std::chrono::microseconds>(t_stop - this->t_start).count();
    auto time_duration_sec = time_duration * 1e-6;

    for (size_t f = 0; f < this->get_n_frames(); f++)
        this->proxy_probe((void*)&time_duration_sec, frame_id);
}

void
Probe_time::reset()
{
    this->t_start = std::chrono::steady_clock::now();
}
