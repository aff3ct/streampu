#include <utility>

#include "Tools/Exception/cannot_allocate/cannot_allocate.hpp"

using namespace aff3ct::tools;

#define DEFAULT_MESSAGE "Cannot allocate the object."

cannot_allocate::cannot_allocate() noexcept
  : exception(DEFAULT_MESSAGE)
{
}

cannot_allocate::cannot_allocate(std::string&& message) noexcept
  : exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

cannot_allocate::cannot_allocate(std::string&& filename,
                                 int&& line_num,
                                 std::string&& funcname,
                                 std::string&& message) noexcept
  : exception(std::move(filename),
              std::move(line_num),
              std::move(funcname),
              message.empty() ? DEFAULT_MESSAGE : message)
{
}