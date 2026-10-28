/*!
 * \file
 * \brief Class tools::out_of_range.
 */
#ifndef OUT_OF_RANGE_HPP_
#define OUT_OF_RANGE_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace spu
{
namespace tools
{
class SPU_EXPORT_EXCEPTION out_of_range : public exception
{
  public:
    out_of_range() noexcept;
    explicit out_of_range(std::string&& message) noexcept;
    out_of_range(std::string&& filename,
                 int&& line_num,
                 std::string&& funcname = "",
                 std::string&& message = "") noexcept;

    virtual ~out_of_range() = default;
};
}
}

#endif /* OUT_OF_RANGE_HPP_ */
