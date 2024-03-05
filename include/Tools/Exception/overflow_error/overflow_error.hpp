/*!
 * \file
 * \brief Class tools::overflow_error.
 */
#ifndef OVERFLOW_ERROR_HPP_
#define OVERFLOW_ERROR_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace aff3ct
{
namespace tools
{
class overflow_error : public exception
{
public:
	overflow_error() noexcept;
	explicit overflow_error(std::string &&message) noexcept;
	overflow_error(std::string &&filename, int &&line_num, std::string &&funcname = "", std::string &&message = "") noexcept;

	virtual ~overflow_error() = default;
};
}
}

#endif /* OVERFLOW_ERROR_HPP_ */
