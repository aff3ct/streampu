/*!
 * \file
 * \brief Class tools::exception.
 */
#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <exception>
#include <string>
#ifdef AFF3CT_CORE_STACKTRACE
#include <cpptrace/cpptrace.hpp>
#endif

namespace aff3ct
{
namespace tools
{
class exception : public std::exception
{
public:
	static bool no_stacktrace;

protected:
	static const std::string empty_string;
	static       std::string messagebuff;

private:
	std::string message;   // the message only
#ifdef AFF3CT_CORE_STACKTRACE
	cpptrace::raw_trace raw_trace; // raw stack trace
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
#include "Tools/Exception/control_flow_error/control_flow_error.hpp"
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
