#include <utility>

#include "Tools/Exception/control_flow_error/control_flow_error.hpp"

using namespace spu::tools;

#define DEFAULT_MESSAGE "Control flow error."

control_flow_error::control_flow_error() noexcept
  : exception(DEFAULT_MESSAGE)
{
}

control_flow_error::control_flow_error(std::string&& message) noexcept
  : exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

control_flow_error::control_flow_error(std::string&& filename,
                                       int&& line_num,
                                       std::string&& funcname,
                                       std::string&& message) noexcept
  : exception(std::move(filename),
              std::move(line_num),
              std::move(funcname),
              message.empty() ? DEFAULT_MESSAGE : message)
{
}