/*!
 * \file
 * \brief Class tools::cannot_allocate.
 */
#ifndef CANNOT_ALLOCATE_HPP_
#define CANNOT_ALLOCATE_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace spu
{
namespace tools
{
class cannot_allocate : public exception
{
  public:
    cannot_allocate() noexcept;
    explicit cannot_allocate(std::string&& message) noexcept;
    cannot_allocate(std::string&& filename,
                    int&& line_num,
                    std::string&& funcname = "",
                    std::string&& message = "") noexcept;

    virtual ~cannot_allocate() = default;
};
}
}

#endif /* CANNOT_ALLOCATE_HPP_ */
