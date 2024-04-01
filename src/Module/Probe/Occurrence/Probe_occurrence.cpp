#include <ios>
#include <sstream>

#include "Module/Probe/Occurrence/Probe_occurrence.hpp"
#include "Tools/Exception/exception.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

Probe_occurrence::Probe_occurrence(const std::string& col_name, tools::Reporter_probe* reporter)
  : Probe<uint8_t>(0, col_name)
  , occurrences(0)
{
    const std::string name = "Probe_occurrence<" + col_name + ">";
    this->set_name(name);

    if (reporter != nullptr) this->register_reporter(reporter);
}

void
Probe_occurrence::register_reporter(tools::Reporter_probe* reporter)
{
    if (this->reporter != nullptr)
    {
        std::stringstream message;
        message << "It is not possible to register this probe to a new 'tools::Reporter_probe' because it is already "
                << "registered to an other 'tools::Reporter_probe'.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    this->reporter = reporter;
    this->proxy_register_probe(1, typeid(int64_t), "", 100, std::ios_base::scientific, 3);
}

void
Probe_occurrence::_probe(const uint8_t* in, const size_t frame_id)
{
    this->proxy_probe((void*)&occurrences, frame_id);
    this->occurrences++;
}

void
Probe_occurrence::reset()
{
    this->occurrences = 0;
}

int64_t
Probe_occurrence::get_occurrences() const
{
    return this->occurrences;
}
