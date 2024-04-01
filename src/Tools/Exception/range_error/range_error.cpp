#include <utility>

#include "Tools/Exception/range_error/range_error.hpp"

using namespace aff3ct::tools;

#define DEFAULT_MESSAGE "Range error."

range_error::range_error() noexcept
  : exception(DEFAULT_MESSAGE)
{
}

range_error::range_error(std::string&& message) noexcept
  : exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

range_error::range_error(std::string&& filename, int&& line_num, std::string&& funcname, std::string&& message) noexcept
  : exception(std::move(filename),
              std::move(line_num),
              std::move(funcname),
              message.empty() ? DEFAULT_MESSAGE : message)
{
}
