/*!
 * \file
 * \brief Class tools::Interface_clone.
 */
#ifndef INTERFACE_CLONE_HPP__
#define INTERFACE_CLONE_HPP__

namespace spu
{
namespace tools
{
class Interface_clone
{
  public:
    virtual Interface_clone* clone() const = 0;
};

}
}

#endif // INTERFACE_CLONE_HPP__