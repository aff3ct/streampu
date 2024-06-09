#include <utility>

#include "Tools/Exception/underflow_error/underflow_error.hpp"

using namespace spu::tools;

#define DEFAULT_MESSAGE "Underflow error."

underflow_error::underflow_error() noexcept
  : exception(DEFAULT_MESSAGE)
{
}

underflow_error::underflow_error(std::string&& message) noexcept
  : exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

underflow_error::underflow_error(std::string&& filename,
                                 int&& line_num,
                                 std::string&& funcname,
                                 std::string&& message) noexcept
  : exception(std::move(filename),
              std::move(line_num),
              std::move(funcname),
              message.empty() ? DEFAULT_MESSAGE : message)
{
}
