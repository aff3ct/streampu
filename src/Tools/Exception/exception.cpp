#include <utility>

#include "Tools/Exception/exception.hpp"
#include "Tools/system_functions.h"

using namespace spu::tools;

bool spu::tools::exception::no_stacktrace = false;

exception::exception() noexcept
#ifdef SPU_STACKTRACE
  : cpptrace::exception_with_message("")
#else
  : message("")
#endif
{
}

exception::exception(std::string&& message) noexcept
#ifdef SPU_STACKTRACE
  : cpptrace::exception_with_message(std::move(message))
#else
  : message(message)
#endif
{
}

exception::exception(std::string&& filename, int&& line_num, std::string&& funcname, std::string&& message) noexcept
#ifdef SPU_STACKTRACE
  : cpptrace::exception_with_message((!filename.empty() ? "In the '" + filename + "' file" : "") +
                                     (line_num >= 0 ? " at line " + std::to_string(line_num) : "") +
                                     (!funcname.empty() ? " ('" + funcname + "' function)" : "") + ": " + "\"" +
                                     message + "\"")
#else
  : message((!filename.empty() ? "In the '" + filename + "' file" : "") +
            (line_num >= 0 ? " at line " + std::to_string(line_num) : "") +
            (!funcname.empty() ? " ('" + funcname + "' function)" : "") + ": " + "\"" + message + "\"")
#endif
{
}

const char*
exception::what() const noexcept
{
#ifdef SPU_STACKTRACE
    if (no_stacktrace)
    {
        return this->message();
    }
    else
    {
#ifdef SPU_COLORS
        if (what_string.empty())
        {
            const bool enable_color = true;
            what_string = message() + std::string(":\n") + this->trace().to_string(enable_color);
        }
        return what_string.c_str();
#else
        return cpptrace::exception_with_message::what();
#endif
    }
#else
    return message.c_str();
#endif
}