/*!
 * \file
 * \brief Class tools::Interface_reset.
 */
#ifndef INTERFACE_RESET_HPP__
#define INTERFACE_RESET_HPP__

namespace spu
{
namespace tools
{
class Interface_reset
{
  public:
    virtual void reset() = 0;
    virtual ~Interface_reset() = default;
};

}
}

#endif // INTERFACE_RESET_HPP__