/*!
 * \file
 * \brief Class module::Source_user.
 */
#ifndef SOURCE_USER_HPP_
#define SOURCE_USER_HPP_

#include <string>
#include <vector>

#include "Module/Stateful/Source/Source.hpp"

namespace spu
{
namespace module
{
template<typename B = int>
class Source_user : public Source<B>
{
  private:
    std::vector<std::vector<B>> source;
    int next_frame_idx;
    int src_counter;
    const bool auto_reset;
    int start_idx;
    bool done;

  public:
    Source_user(const int max_data_size, const std::string& filename, const bool auto_reset, const int start_idx = 0);
    Source_user(const int max_data_size, const std::string& filename, const int start_idx = 0);
    virtual ~Source_user() = default;

    virtual Source_user<B>* clone() const;

    virtual bool is_done() const;

    virtual void reset();

  protected:
    void _generate(B* out_data, const size_t frame_id);
};
}
}

#endif /* SOURCE_USER_HPP_ */
