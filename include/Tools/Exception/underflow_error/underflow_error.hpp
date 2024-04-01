/*!
 * \file
 * \brief Class tools::underflow_error.
 */
#ifndef UNDERFLOW_ERROR_HPP_
#define UNDERFLOW_ERROR_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace aff3ct
{
namespace tools
{
class underflow_error : public exception
{
  public:
    underflow_error() noexcept;
    explicit underflow_error(std::string&& message) noexcept;
    underflow_error(std::string&& filename,
                    int&& line_num,
                    std::string&& funcname = "",
                    std::string&& message = "") noexcept;

    virtual ~underflow_error() = default;
};
}
}

#endif /* UNDERFLOW_ERROR_HPP_ */
