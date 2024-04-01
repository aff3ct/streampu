#ifndef COMPUTE_BYTES_H_
#define COMPUTE_BYTES_H_

#include <cstddef>
#include <typeindex>
#include <vector>

namespace aff3ct
{
namespace tools
{
size_t
compute_bytes(const size_t n_elmts, const std::type_index type);

std::vector<size_t>
compute_bytes(const std::vector<size_t>& n_elmts, const std::vector<std::type_index>& type);
}
}

#endif /* COMPUTE_BYTES_H_*/
