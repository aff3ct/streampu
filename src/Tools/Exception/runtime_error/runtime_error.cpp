#include <utility>

#include "Tools/Exception/runtime_error/runtime_error.hpp"

using namespace spu::tools;

#define DEFAULT_MESSAGE "Runtime error."

runtime_error::runtime_error() noexcept
  : exception(DEFAULT_MESSAGE)
{
}

runtime_error::runtime_error(std::string&& message) noexcept
  : exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

runtime_error::runtime_error(std::string&& filename,
                             int&& line_num,
                             std::string&& funcname,
                             std::string&& message) noexcept
  : exception(std::move(filename),
              std::move(line_num),
              std::move(funcname),
              message.empty() ? DEFAULT_MESSAGE : message)
{
}