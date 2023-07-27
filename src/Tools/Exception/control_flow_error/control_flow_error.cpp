#include "Tools/Exception/control_flow_error/control_flow_error.hpp"

using namespace aff3ct::tools;

const std::string control_flow_error::default_message = "control flow error.";

control_flow_error
::control_flow_error() throw()
: exception()
{
}

control_flow_error
::control_flow_error(const std::string &message) throw()
: exception(message.empty() ? default_message : message)
{
}

control_flow_error
::control_flow_error(const std::string &filename,
                     const int line_num,
                     const std::string &funcname,
                     const std::string &message) throw()
: exception(filename, line_num, funcname, message.empty() ? default_message : message)
{
}