#include <ios>
#include <sstream>

#include "Module/Probe/Value/Probe_value.hpp"
#include "Tools/Exception/exception.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

template<typename T>
Probe_value<T>::Probe_value(const int size, const std::string& col_name, tools::Reporter_probe* reporter)
  : Probe<T>(size, col_name)
{
    const std::string name = "Probe_value<" + col_name + ">";
    this->set_name(name);

    if (size <= 0)
    {
        std::stringstream message;
        message << "'size' has to be greater than 0 ('size' = " << size << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (reporter != nullptr) this->register_reporter(reporter);
}

template<typename T>
void
Probe_value<T>::register_reporter(tools::Reporter_probe* reporter)
{
    if (this->reporter != nullptr)
    {
        std::stringstream message;
        message << "It is not possible to register this probe to a new 'tools::Reporter_probe' because it is already "
                << "registered to an other 'tools::Reporter_probe'.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    this->reporter = reporter;
    this->proxy_register_probe(this->get_socket_size(), typeid(T), "", 100, std::ios_base::scientific, 3);
}

template<typename T>
void
Probe_value<T>::_probe(const T* in, const size_t frame_id)
{
    this->proxy_probe((void*)in, frame_id);
}

// ==================================================================================== explicit template instantiation
template class aff3ct::module::Probe_value<int8_t>;
template class aff3ct::module::Probe_value<uint8_t>;
template class aff3ct::module::Probe_value<int16_t>;
template class aff3ct::module::Probe_value<uint16_t>;
template class aff3ct::module::Probe_value<int32_t>;
template class aff3ct::module::Probe_value<uint32_t>;
template class aff3ct::module::Probe_value<int64_t>;
template class aff3ct::module::Probe_value<uint64_t>;
template class aff3ct::module::Probe_value<float>;
template class aff3ct::module::Probe_value<double>;
// ==================================================================================== explicit template instantiation
