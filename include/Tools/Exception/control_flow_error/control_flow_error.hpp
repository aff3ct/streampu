/*!
 * \file
 * \brief Class tools::control_flow_error.
 */
#ifndef CONTROL_FLOW_ERROR_HPP_
#define CONTROL_FLOW_ERROR_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace aff3ct
{
namespace tools
{
class control_flow_error : public exception
{
  public:
    control_flow_error() noexcept;
    explicit control_flow_error(std::string&& message) noexcept;
    control_flow_error(std::string&& filename,
                       int&& line_num,
                       std::string&& funcname = "",
                       std::string&& message = "") noexcept;

    virtual ~control_flow_error() = default;
};
}
}

#endif /* CONTROL_FLOW_ERROR_HPP_ */
