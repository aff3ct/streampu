#include <utility>

#include "Tools/Exception/processing_aborted/processing_aborted.hpp"

using namespace aff3ct::tools;

#define DEFAULT_MESSAGE "Processing aborted."

processing_aborted
::processing_aborted() noexcept
: exception(DEFAULT_MESSAGE)
{
}

processing_aborted
::processing_aborted(std::string &&message) noexcept
: exception(message.empty() ? DEFAULT_MESSAGE : message)
{
}

processing_aborted
::processing_aborted(std::string &&filename, int &&line_num, std::string &&funcname, std::string &&message) noexcept
: exception(std::move(filename), std::move(line_num), std::move(funcname), message.empty() ? DEFAULT_MESSAGE : message)
{
}
