/*!
 * \file
 * \brief Class tools::processing_aborted.
 */
#ifndef PROCESSING_ABORTED_HPP_
#define PROCESSING_ABORTED_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace aff3ct
{
namespace tools
{
class processing_aborted : public exception
{
  public:
    processing_aborted() noexcept;
    explicit processing_aborted(std::string&& message) noexcept;
    processing_aborted(std::string&& filename,
                       int&& line_num,
                       std::string&& funcname = "",
                       std::string&& message = "") noexcept;

    virtual ~processing_aborted() = default;
};
}
}

#endif /* PROCESSING_ABORTED_HPP_ */
