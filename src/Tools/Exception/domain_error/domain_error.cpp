#include <utility>

#include "Tools/Exception/domain_error/domain_error.hpp"

using namespace spu::tools;

#define DEFAULT_MESSAGE "Domain error."

domain_error::domain_error() noexcept
  : exception(DEFAULT_MESSAGE)
{
}

domain_error::domain_error(std::string&& message) noexcept
  : exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

domain_error::domain_error(std::string&& filename,
                           int&& line_num,
                           std::string&& funcname,
                           std::string&& message) noexcept
  : exception(std::move(filename),
              std::move(line_num),
              std::move(funcname),
              message.empty() ? DEFAULT_MESSAGE : message)
{
}