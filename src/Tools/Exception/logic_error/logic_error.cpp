#include <utility>

#include "Tools/Exception/logic_error/logic_error.hpp"

using namespace spu::tools;

#define DEFAULT_MESSAGE "Logic error."

logic_error::logic_error() noexcept
  : exception(DEFAULT_MESSAGE)
{
}

logic_error::logic_error(std::string&& message) noexcept
  : exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

logic_error::logic_error(std::string&& filename, int&& line_num, std::string&& funcname, std::string&& message) noexcept
  : exception(std::move(filename),
              std::move(line_num),
              std::move(funcname),
              message.empty() ? DEFAULT_MESSAGE : message)
{
}