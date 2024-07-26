#include <ios>
#include <sstream>

#include "Module/Stateful/Probe/Throughput/Probe_throughput.hpp"
#include "Tools/Exception/exception.hpp"

using namespace spu;
using namespace spu::module;

Probe_throughput::Probe_throughput(const size_t data_size,
                                   const std::string& col_name,
                                   const double factor,
                                   tools::Reporter_probe* reporter)
  : Probe<uint8_t>(0, col_name)
  , t_start(std::chrono::steady_clock::now())
  , data_size(data_size)
  , thr(0.)
  , factor(factor)
{
    const std::string name = "Probe_throughput<" + col_name + ">";
    this->set_name(name);
    this->set_single_wave(true);

    if (reporter != nullptr) this->register_reporter(reporter);
}

Probe_throughput::Probe_throughput(const size_t data_size, const std::string& col_name, tools::Reporter_probe* reporter)
  : Probe_throughput(data_size, col_name, 1024. * 1024., reporter)
{
}

Probe_throughput::Probe_throughput(const std::string& col_name, tools::Reporter_probe* reporter)
  : Probe_throughput(1, col_name, reporter)
{
}

void
Probe_throughput::register_reporter(tools::Reporter_probe* reporter)
{
    if (this->reporter != nullptr)
    {
        std::stringstream message;
        message << "It is not possible to register this probe to a new 'tools::Reporter_probe' because it is already "
                << "registered to an other 'tools::Reporter_probe'.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    this->reporter = reporter;
    this->proxy_register_probe(1, typeid(double), "", 100, std::ios_base::dec | std::ios_base::fixed, 3);
}

void
Probe_throughput::_probe(const uint8_t* in, const size_t frame_id)
{
    auto t_stop = std::chrono::steady_clock::now();
    auto time_duration = (double)std::chrono::duration_cast<std::chrono::microseconds>(t_stop - this->t_start).count();
    this->thr = ((double)(this->data_size * this->get_n_frames()) / (this->factor)) / (time_duration * 1e-6);
    this->t_start = t_stop;

    for (size_t f = 0; f < this->get_n_frames(); f++)
        this->proxy_probe((void*)&thr, frame_id);
}

void
Probe_throughput::reset()
{
    this->t_start = std::chrono::steady_clock::now();
    this->thr = 0.;
}
