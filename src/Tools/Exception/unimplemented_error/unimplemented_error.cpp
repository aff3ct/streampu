#include <utility>

#include "Tools/Exception/unimplemented_error/unimplemented_error.hpp"

using namespace aff3ct::tools;

#define DEFAULT_MESSAGE "Unimplemented function or method."

unimplemented_error::unimplemented_error() noexcept
  : exception(DEFAULT_MESSAGE)
{
}

unimplemented_error::unimplemented_error(std::string&& message) noexcept
  : exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

unimplemented_error::unimplemented_error(std::string&& filename,
                                         int&& line_num,
                                         std::string&& funcname,
                                         std::string&& message) noexcept
  : exception(std::move(filename),
              std::move(line_num),
              std::move(funcname),
              message.empty() ? DEFAULT_MESSAGE : message)
{
}
