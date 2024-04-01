/*!
 * \file
 * \brief Class tools::runtime_error.
 */
#ifndef RUNTIME_ERROR_HPP_
#define RUNTIME_ERROR_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace aff3ct
{
namespace tools
{
class runtime_error : public exception
{
  public:
    runtime_error() noexcept;
    explicit runtime_error(std::string&& message) noexcept;
    runtime_error(std::string&& filename,
                  int&& line_num,
                  std::string&& funcname = "",
                  std::string&& message = "") noexcept;

    virtual ~runtime_error() = default;
};
}
}

#endif /* RUNTIME_ERROR_HPP_ */
