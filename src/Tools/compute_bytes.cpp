#include <cstdint>
#include <sstream>

#include "Tools/Exception/exception.hpp"
#include "Tools/compute_bytes.h"

size_t
spu::tools::compute_bytes(const size_t n_elmts, const std::type_index type)
{
    if (type == typeid(int8_t))
        return n_elmts * sizeof(int8_t);
    else if (type == typeid(uint8_t))
        return n_elmts * sizeof(uint8_t);
    else if (type == typeid(int16_t))
        return n_elmts * sizeof(int16_t);
    else if (type == typeid(uint16_t))
        return n_elmts * sizeof(uint16_t);
    else if (type == typeid(int32_t))
        return n_elmts * sizeof(int32_t);
    else if (type == typeid(uint32_t))
        return n_elmts * sizeof(uint32_t);
    else if (type == typeid(int64_t))
        return n_elmts * sizeof(int64_t);
    else if (type == typeid(uint64_t))
        return n_elmts * sizeof(uint64_t);
    else if (type == typeid(float))
        return n_elmts * sizeof(float);
    else if (type == typeid(double))
        return n_elmts * sizeof(double);
    else
    {
        std::stringstream message;
        message << "This should never happen.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
}

std::vector<size_t>
spu::tools::compute_bytes(const std::vector<size_t>& n_elmts, const std::vector<std::type_index>& type)
{
    if (n_elmts.size() != type.size())
    {
        std::stringstream message;
        message << "'n_elmts.size()' has to be equal to 'type.size()' ('n_elmts.size()' = " << n_elmts.size()
                << ", 'type.size()' = " << type.size() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    std::vector<size_t> bytes(n_elmts.size());
    for (size_t i = 0; i < n_elmts.size(); i++)
        bytes[i] = spu::tools::compute_bytes(n_elmts[i], type[i]);
    return bytes;
}
