/*!
 * \file
 * \brief Class module::Source_AZCW.
 */
#ifndef SOURCE_AZCW_HPP_
#define SOURCE_AZCW_HPP_

#include "Module/Source/Source.hpp"

namespace spu
{
namespace module
{
template<typename B = int>
class Source_AZCW : public Source<B>
{
  public:
    Source_AZCW(const int max_data_size);

    virtual ~Source_AZCW() = default;

    virtual Source_AZCW<B>* clone() const;

  protected:
    void _generate(B* out_data, const size_t frame_id);
};
}
}

#endif /* SOURCE_AZCW_HPP_ */
