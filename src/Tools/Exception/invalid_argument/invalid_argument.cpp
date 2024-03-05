#include <utility>

#include "Tools/Exception/invalid_argument/invalid_argument.hpp"

using namespace aff3ct::tools;

#define DEFAULT_MESSAGE "Given argument is invalid."

invalid_argument
::invalid_argument() noexcept
: exception(DEFAULT_MESSAGE)
{
}

invalid_argument
::invalid_argument(std::string &&message) noexcept
: exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

invalid_argument
::invalid_argument(std::string &&filename, int &&line_num, std::string &&funcname, std::string &&message) noexcept
: exception(std::move(filename), std::move(line_num), std::move(funcname), message.empty() ? DEFAULT_MESSAGE : message)
{
}