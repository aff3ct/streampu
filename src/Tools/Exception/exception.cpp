#include <utility>

#include "Tools/system_functions.h"
#include "Tools/Exception/exception.hpp"

using namespace aff3ct::tools;

bool aff3ct::tools::exception::no_stacktrace = false;

exception
::exception() noexcept
#ifdef AFF3CT_CORE_STACKTRACE
: cpptrace::exception_with_message("")
#else
: message("")
#endif
{
}

exception
::exception(std::string&& message) noexcept
#ifdef AFF3CT_CORE_STACKTRACE
: cpptrace::exception_with_message(std::move(message))
#else
: message(message)
#endif
{
}

exception
::exception(std::string &&filename, int &&line_num, std::string &&funcname, std::string &&message) noexcept
#ifdef AFF3CT_CORE_STACKTRACE
: cpptrace::exception_with_message(
  (!filename.empty() ? "In the '" + filename + "' file"       : "") +
  (line_num >= 0     ? " at line " + std::to_string(line_num) : "") +
  (!funcname.empty() ? " ('" + funcname + "' function)"       : "") +
  ": " + "\"" + message + "\"")
#else
: message((!filename.empty() ? "In the '" + filename + "' file"       : "") +
          (line_num >= 0     ? " at line " + std::to_string(line_num) : "") +
          (!funcname.empty() ? " ('" + funcname + "' function)"       : "") +
          ": " + "\"" + message + "\"")
#endif
{
}

const char* exception
::what() const noexcept
{
#ifdef AFF3CT_CORE_STACKTRACE
	if (no_stacktrace)
		return this->message();
	else
		return cpptrace::exception_with_message::what();
#else
	return message.c_str();
#endif
}