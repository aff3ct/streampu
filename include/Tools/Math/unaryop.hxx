#include <cmath> // fabs(), copysign()...

#include "Tools/Math/unaryop.h"

namespace spu
{
namespace tools
{
template<typename TI, typename TO>
inline TO
uop_cast(const TI a)
{
    return (TO)a;
}

template<typename TI, typename TO>
inline TO
uop_neg(const TI a)
{
    return (TO)-a;
}

template<typename TI, typename TO>
inline TO
uop_abs(const TI a)
{
    return (TO)std::abs(a);
}

template<typename TI, typename TO>
inline TO
uop_not(const TI a)
{
    return (TO)~a;
}

template<typename TI, typename TO>
inline TO
uop_not_abs(const TI a)
{
    return (TO)~std::abs(a);
}

template<typename TI, typename TO>
inline TO
uop_sign(const TI a)
{
    return (TO)std::signbit(a);
}

template<typename TI, typename TO, proto_uop<TI, TO> UOP>
std::string
uop_get_name()
{
    std::string op = "ukn";
#ifndef _MSC_VER
    if ((uintptr_t)UOP == (uintptr_t)uop_cast<TI, TO>) op = "cast";
#ifdef __cpp_if_constexpr
    if constexpr (std::is_signed_v<TI>)
    {
        if ((uintptr_t)UOP == (uintptr_t)uop_sign<TI, TO>)
            op = "sign";
        else if ((uintptr_t)UOP == (uintptr_t)uop_neg<TI, TO>)
            op = "neg";

        if constexpr (std::is_integral_v<TI>)
            if ((uintptr_t)UOP == (uintptr_t)uop_not_abs<TI, TO>) op = "not_abs";
    }
    if constexpr (std::is_floating_point_v<TI>)
    {
        if ((uintptr_t)UOP == (uintptr_t)uop_abs<TI, TO>) op = "abs";
    }
    else if constexpr (std::is_integral_v<TI>)
    {
        if ((uintptr_t)UOP == (uintptr_t)uop_not<TI, TO>) op = "not";
    }
#endif
#endif
    return op;
}

}
}
