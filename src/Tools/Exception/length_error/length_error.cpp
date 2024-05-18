#include <utility>

#include "Tools/Exception/length_error/length_error.hpp"

using namespace spu::tools;

#define DEFAULT_MESSAGE "Length error."

length_error::length_error() noexcept
  : exception(DEFAULT_MESSAGE)
{
}

length_error::length_error(std::string&& message) noexcept
  : exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

length_error::length_error(std::string&& filename,
                           int&& line_num,
                           std::string&& funcname,
                           std::string&& message) noexcept
  : exception(std::move(filename),
              std::move(line_num),
              std::move(funcname),
              message.empty() ? DEFAULT_MESSAGE : message)
{
}