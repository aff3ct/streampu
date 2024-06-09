/*!
 * \file
 * \brief Class tools::exception.
 */
#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <exception>
#include <string>
#ifdef SPU_STACKTRACE
#include <cpptrace/cpptrace.hpp>
#endif

namespace spu
{
namespace tools
{
#ifdef SPU_STACKTRACE
class exception : public cpptrace::exception_with_message
{
#else
class exception : public std::exception
{
#endif
  public:
    static bool no_stacktrace;

  private:
#ifdef SPU_STACKTRACE
    mutable std::string what_string;
#else
    std::string message; // the message only
#endif

  public:
    exception() noexcept;
    explicit exception(std::string&& message) noexcept;
    exception(std::string&& filename, int&& line_num, std::string&& funcname = "", std::string&& message = "") noexcept;

    virtual ~exception() = default;

    const char* what() const noexcept override; // return the message and the back trace if enabled
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
#include "Tools/Exception/processing_aborted/processing_aborted.hpp"
#include "Tools/Exception/range_error/range_error.hpp"
#include "Tools/Exception/runtime_error/runtime_error.hpp"
#include "Tools/Exception/underflow_error/underflow_error.hpp"
#include "Tools/Exception/unimplemented_error/unimplemented_error.hpp"
#include "Tools/Exception/waiting_canceled/waiting_canceled.hpp"

#endif /* EXCEPTION_HPP_ */
