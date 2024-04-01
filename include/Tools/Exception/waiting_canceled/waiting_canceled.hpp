/*!
 * \file
 * \brief Class tools::waiting_canceled.
 */
#ifndef WAITING_CANCELED_HPP_
#define WAITING_CANCELED_HPP_

#include <string>

#include "Tools/Exception/exception.hpp"

namespace aff3ct
{
namespace tools
{
class waiting_canceled : public exception
{
  public:
    waiting_canceled() noexcept;
    explicit waiting_canceled(std::string&& message) noexcept;
    waiting_canceled(std::string&& filename,
                     int&& line_num,
                     std::string&& funcname = "",
                     std::string&& message = "") noexcept;

    virtual ~waiting_canceled() = default;
};
}
}

#endif /* WAITING_CANCELED_HPP_ */
