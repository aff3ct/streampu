#include "Tools/system_functions.h"
#include "Tools/Exception/exception.hpp"

using namespace aff3ct::tools;

const std::string aff3ct::tools::exception::empty_string = "";
bool aff3ct::tools::exception::no_stacktrace = false;
std::string aff3ct::tools::exception::messagebuff;

exception
::exception() throw()
{
}

exception
::exception(const std::string &message) throw()
: message(message)
#ifdef AFF3CT_CORE_STACKTRACE
, raw_trace(cpptrace::generate_raw_trace(2))
#endif
{
}

exception
::exception(const std::string &filename,
            const int line_num,
            const std::string &funcname,
            const std::string &_message) throw()
{
	if (!filename.empty())
		this->message += "In the '" + filename + "' file";
	if (line_num >= 0)
		this->message += " at line " + std::to_string(line_num);
	if (!funcname.empty())
		this->message += " ('" + funcname + "' function)";
	this->message += ": ";
	this->message += "\"" + _message + "\"";

#ifdef AFF3CT_CORE_STACKTRACE
	raw_trace = cpptrace::generate_raw_trace(2);
#endif
}

const char* exception
::what() const throw()
{
#ifdef AFF3CT_CORE_STACKTRACE
	if (no_stacktrace)
	{
		return message.c_str();
	}
	else
	{
#ifdef AFF3CT_CORE_COLORS
		std::string stacktrace_str = raw_trace.resolve().to_string(true);
#else
		std::string stacktrace_str = raw_trace.resolve().to_string(false);
#endif
		messagebuff = message + "\n" + stacktrace_str;
		return messagebuff.c_str();
	}
#else
	return message.c_str();
#endif
}