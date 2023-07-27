/*!
 * \file
 * \brief Class tools::control_flow_error.
 */
#ifndef CONTROL_FLOW_ERROR_HPP_
#define CONTROL_FLOW_ERROR_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace aff3ct
{
namespace tools
{
class control_flow_error : public exception
{
	static const std::string default_message;

public:
	control_flow_error() throw();

	explicit control_flow_error(const std::string &message) throw();

	control_flow_error(const std::string &filename,
	                   const int line_num,
	                   const std::string &funcname = exception::empty_string,
	                   const std::string &message = exception::empty_string) throw();

	virtual ~control_flow_error() = default;
};
}
}

#endif /* CONTROL_FLOW_ERROR_HPP_ */
