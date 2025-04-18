/*!
 * \file
 * \brief Class tools::range_error.
 */
#ifndef RANGE_ERROR_HPP_
#define RANGE_ERROR_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace spu
{
namespace tools
{
class SPU_EXPORT_EXCEPTION range_error : public exception
{
  public:
    range_error() noexcept;
    explicit range_error(std::string&& message) noexcept;
    range_error(std::string&& filename,
                int&& line_num,
                std::string&& funcname = "",
                std::string&& message = "") noexcept;

    virtual ~range_error() = default;
};
}
}

#endif /* RANGE_ERROR_HPP_ */
