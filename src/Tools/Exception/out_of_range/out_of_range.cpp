#include <utility>

#include "Tools/Exception/out_of_range/out_of_range.hpp"

using namespace aff3ct::tools;

#define DEFAULT_MESSAGE "Out of range."

out_of_range::out_of_range() noexcept
  : exception(DEFAULT_MESSAGE)
{
}

out_of_range::out_of_range(std::string&& message) noexcept
  : exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

out_of_range::out_of_range(std::string&& filename,
                           int&& line_num,
                           std::string&& funcname,
                           std::string&& message) noexcept
  : exception(std::move(filename),
              std::move(line_num),
              std::move(funcname),
              message.empty() ? DEFAULT_MESSAGE : message)
{
}