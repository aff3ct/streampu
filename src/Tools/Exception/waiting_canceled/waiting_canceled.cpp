#include <utility>

#include "Tools/Exception/waiting_canceled/waiting_canceled.hpp"

using namespace aff3ct::tools;

#define DEFAULT_MESSAGE "Waiting canceled thread."

waiting_canceled
::waiting_canceled() noexcept
: exception(DEFAULT_MESSAGE)
{
}

waiting_canceled
::waiting_canceled(std::string &&message) noexcept
: exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

waiting_canceled
::waiting_canceled(std::string &&filename, int &&line_num, std::string &&funcname, std::string &&message) noexcept
: exception(std::move(filename), std::move(line_num), std::move(funcname), message.empty() ? DEFAULT_MESSAGE : message)
{
}
