#include <utility>

#include "Tools/Exception/overflow_error/overflow_error.hpp"

using namespace aff3ct::tools;

#define DEFAULT_MESSAGE "Overflow error."

overflow_error
::overflow_error() noexcept
: exception(DEFAULT_MESSAGE)
{
}

overflow_error
::overflow_error(std::string &&message) noexcept
: exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

overflow_error
::overflow_error(std::string &&filename, int &&line_num, std::string &&funcname, std::string &&message) noexcept
: exception(std::move(filename), std::move(line_num), std::move(funcname), message.empty() ? DEFAULT_MESSAGE : message)
{
}