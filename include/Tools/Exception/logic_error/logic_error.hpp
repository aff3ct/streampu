/*!
 * \file
 * \brief Class tools::logic_error.
 */
#ifndef LOGIC_ERROR_HPP_
#define LOGIC_ERROR_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace aff3ct
{
namespace tools
{
class logic_error : public exception
{
  public:
    logic_error() noexcept;
    explicit logic_error(std::string&& message) noexcept;
    logic_error(std::string&& filename,
                int&& line_num,
                std::string&& funcname = "",
                std::string&& message = "") noexcept;

    virtual ~logic_error() = default;
};
}
}

#endif /* LOGIC_ERROR_HPP_ */
