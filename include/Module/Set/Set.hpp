/*!
 * \file
 * \brief Class module::Set.
 */
#ifndef SET_HPP_
#define SET_HPP_

#include <cstddef>
#include <memory>

#include "Module/Module.hpp"

namespace aff3ct
{
namespace runtime
{
class Sequence;
}
namespace module
{
namespace set
{
enum class tsk : size_t
{
    exec,
    SIZE
};
}

class Set : public Module
{
    std::shared_ptr<runtime::Sequence> sequence_cloned;
    runtime::Sequence* sequence_extern;

  public:
    inline runtime::Task& operator[](const set::tsk t);

    explicit Set(const runtime::Sequence& sequence);
    explicit Set(runtime::Sequence& sequence);
    virtual ~Set() = default;

    virtual void init();

    virtual Set* clone() const;

    runtime::Sequence& get_sequence();

    virtual void set_n_frames(const size_t n_frames);

  protected:
    using Module::deep_copy;
    virtual void deep_copy(const Set& m);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Set/Set.hxx"
#endif

#endif /* SET_HPP_ */
