#include "Module/Probe/Probe.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

AProbe::AProbe()
  : Module()
  , reporter(nullptr)
{
}

void
AProbe::check_reporter()
{
    if (reporter == nullptr)
    {
        std::stringstream message;
        message << "'reporter' can't be null, it is required to call 'AProbe::register_reporter()' first.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
}

void
AProbe::proxy_register_probe(const size_t data_size,
                             const std::type_index data_type,
                             const std::string& unit,
                             const size_t buffer_size,
                             const std::ios_base::fmtflags ff,
                             const size_t precision)
{
    this->reporter->register_probe(*this, data_size, data_type, unit, buffer_size, ff, precision);
}

void
AProbe::proxy_probe(const void* data, const size_t frame_id)
{
    this->check_reporter();
    this->reporter->probe(*this, data, frame_id);
}
