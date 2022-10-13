/*!
 * \file
 * \brief Class tools::exception.
 */
#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <exception>
#include <string>

namespace aff3ct
{
namespace tools
{
class exception : public std::exception
{
public:
	static bool no_backtrace;
	static bool no_addr_to_line;

protected:
	static const std::string empty_string;

private:
	std::string message;   // the message only
#ifdef AFF3CT_CORE_BACKTRACE
	std::string backtrace; // the message + the backtrace
	std::string backtrace_a2l; // the message + the backtrace with addr_to_line conversion
#endif

public:
	exception() throw();

	explicit exception(const std::string &message) throw();

	exception(const std::string &filename,
	          const int line_num,
	          const std::string &funcname,
	          const std::string &message) throw();

	virtual ~exception() = default;

	virtual const char* what() const throw(); // return the message and the back trace if enabled
};
}
}

// include specific exceptions
#include "Tools/Exception/cannot_allocate/cannot_allocate.hpp"
#include "Tools/Exception/domain_error/domain_error.hpp"
#include "Tools/Exception/invalid_argument/invalid_argument.hpp"
#include "Tools/Exception/length_error/length_error.hpp"
#include "Tools/Exception/logic_error/logic_error.hpp"
#include "Tools/Exception/out_of_range/out_of_range.hpp"
#include "Tools/Exception/overflow_error/overflow_error.hpp"
#include "Tools/Exception/range_error/range_error.hpp"
#include "Tools/Exception/runtime_error/runtime_error.hpp"
#include "Tools/Exception/underflow_error/underflow_error.hpp"
#include "Tools/Exception/unimplemented_error/unimplemented_error.hpp"
#include "Tools/Exception/waiting_canceled/waiting_canceled.hpp"
#include "Tools/Exception/processing_aborted/processing_aborted.hpp"

#endif /* EXCEPTION_HPP_ */
