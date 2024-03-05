/*!
 * \file
 * \brief Class tools::unimplemented_error.
 */
#ifndef UNIMPLEMENTED_ERROR_HPP_
#define UNIMPLEMENTED_ERROR_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace aff3ct
{
namespace tools
{
class unimplemented_error : public exception
{
public:
	unimplemented_error() noexcept;
	explicit unimplemented_error(std::string &&message) noexcept;
	unimplemented_error(std::string &&filename, int &&line_num, std::string &&funcname = "", std::string &&message = "") noexcept;

	virtual ~unimplemented_error() = default;
};
}
}

#endif /* UNIMPLEMENTED_ERROR_HPP_ */
