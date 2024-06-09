/*!
 * \file
 * \brief Class module::Unaryop.
 */
#ifndef UNARYOP_HPP_
#define UNARYOP_HPP_

#include <cstdint>
#include <memory>
#include <vector>

#include "Module/Module.hpp"
#include "Runtime/Socket/Socket.hpp"
#include "Runtime/Task/Task.hpp"
#include "Tools/Math/unaryop.h"

namespace spu
{

namespace module
{
namespace uop
{
enum class tsk : size_t
{
    perform,
    SIZE
};

namespace sck
{
enum class perform : size_t
{
    in,
    out,
    status
};
}
}

template<typename TI, typename TO, tools::proto_uop<TI, TO> UOP>
class Unaryop : public Module
{
  public:
    inline runtime::Task& operator[](const uop::tsk t);
    inline runtime::Socket& operator[](const uop::sck::perform s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t n_elmts;

  public:
    Unaryop(const size_t n_elmts);
    virtual ~Unaryop() = default;
    virtual Unaryop<TI, TO, UOP>* clone() const;

    size_t get_n_elmts() const;

    template<class AI = std::allocator<TI>, class AO = std::allocator<TO>>
    void perform(const std::vector<TI, AI>& in,
                 std::vector<TO, AO>& out,
                 const int frame_id = -1,
                 const bool managed_memory = true);

    void perform(const TI* in, TO* out, const int frame_id = -1, const bool managed_memory = true);

  protected:
    virtual void _perform(const TI* in, TO* out, const size_t frame_id);
};

#ifdef _MSC_VER // Hack for MSVC compilation /!\ "Unaryop::getname" does not work correctly on MSVC
template<typename TI, typename TO = TI, tools::proto_uop<TI, TO> UOP = tools::uop_abs<TI, TO>>
using Unaryop_abs = Unaryop<TI, TO, UOP>;
template<typename TI, typename TO = TI, tools::proto_uop<TI, TO> UOP = tools::uop_cast<TI, TO>>
using Unaryop_cast = Unaryop<TI, TO, UOP>;
template<typename TI, typename TO = TI, tools::proto_uop<TI, TO> UOP = tools::uop_neg<TI, TO>>
using Unaryop_neg = Unaryop<TI, TO, UOP>;
template<typename TI, typename TO = TI, tools::proto_uop<TI, TO> UOP = tools::uop_not<TI, TO>>
using Unaryop_not = Unaryop<TI, TO, UOP>;
template<typename TI, typename TO = TI, tools::proto_uop<TI, TO> UOP = tools::uop_not_abs<TI, TO>>
using Unaryop_not_abs = Unaryop<TI, TO, UOP>;
template<typename TI, typename TO = TI, tools::proto_uop<TI, TO> UOP = tools::uop_sign<TI, TO>>
using Unaryop_sign = Unaryop<TI, TO, UOP>;
#else // Standard code
template<typename TI, typename TO = TI>
using Unaryop_abs = Unaryop<TI, TO, tools::uop_abs<TI, TO>>;
template<typename TI, typename TO = TI>
using Unaryop_cast = Unaryop<TI, TO, tools::uop_cast<TI, TO>>;
template<typename TI, typename TO = TI>
using Unaryop_neg = Unaryop<TI, TO, tools::uop_neg<TI, TO>>;
template<typename TI, typename TO = TI>
using Unaryop_not = Unaryop<TI, TO, tools::uop_not<TI, TO>>;
template<typename TI, typename TO = TI>
using Unaryop_not_abs = Unaryop<TI, TO, tools::uop_not_abs<TI, TO>>;
template<typename TI, typename TO = TI>
using Unaryop_sign = Unaryop<TI, TO, tools::uop_sign<TI, TO>>;
#endif
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Unaryop/Unaryop.hxx"
#endif

#endif /* UNARYOP_HPP_ */
