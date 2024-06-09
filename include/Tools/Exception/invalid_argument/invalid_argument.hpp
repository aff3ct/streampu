/*!
 * \file
 * \brief Class tools::invalid_argument.
 */
#ifndef INVALID_ARGUMENT_HPP_
#define INVALID_ARGUMENT_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace spu
{
namespace tools
{
class invalid_argument : public exception
{
  public:
    invalid_argument() noexcept;
    explicit invalid_argument(std::string&& message) noexcept;
    invalid_argument(std::string&& filename,
                     int&& line_num,
                     std::string&& funcname = "",
                     std::string&& message = "") noexcept;

    virtual ~invalid_argument() = default;
};
}
}

#endif /* INVALID_ARGUMENT_HPP_ */
