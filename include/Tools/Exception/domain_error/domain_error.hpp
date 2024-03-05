/*!
 * \file
 * \brief Class tools::domain_error.
 */
#ifndef DOMAIN_ERROR_HPP_
#define DOMAIN_ERROR_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace aff3ct
{
namespace tools
{
class domain_error : public exception
{
public:
	domain_error() noexcept;
	explicit domain_error(std::string &&message) noexcept;
	domain_error(std::string &&filename, int &&line_num, std::string &&funcname = "", std::string &&message = "") noexcept;

	virtual ~domain_error() = default;
};
}
}

#endif /* DOMAIN_ERROR_HPP_ */
