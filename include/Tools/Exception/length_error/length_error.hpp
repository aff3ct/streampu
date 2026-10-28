/*!
 * \file
 * \brief Class tools::length_error.
 */
#ifndef LENGTH_ERROR_HPP_
#define LENGTH_ERROR_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace spu
{
namespace tools
{
class SPU_EXPORT_EXCEPTION length_error : public exception
{
  public:
    length_error() noexcept;
    explicit length_error(std::string&& message) noexcept;
    length_error(std::string&& filename,
                 int&& line_num,
                 std::string&& funcname = "",
                 std::string&& message = "") noexcept;

    virtual ~length_error() = default;
};
}
}

#endif /* LENGTH_ERROR_HPP_ */
