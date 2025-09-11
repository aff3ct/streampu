#ifdef SPU_JULIA
#include <julia.h>

#include "Tools/Exception/unimplemented_error/unimplemented_error.hpp"
#include "Tools/Julia/Ref_keeper.hpp"
#include "Tools/Julia/utils.hpp"

namespace spu
{
namespace tools
{

inline jl_value_t*
box(int64_t x)
{
    return jl_box_int64(x);
}

inline jl_value_t*
box(int32_t x)
{
    return jl_box_int32(x);
}

inline jl_value_t*
box(int16_t x)
{
    return jl_box_int16(x);
}

inline jl_value_t*
box(int8_t x)
{
    return jl_box_int8(x);
}

inline jl_value_t*
box(uint64_t x)
{
    return jl_box_uint64(x);
}

inline jl_value_t*
box(uint32_t x)
{
    return jl_box_uint32(x);
}

inline jl_value_t*
box(uint16_t x)
{
    return jl_box_uint16(x);
}

inline jl_value_t*
box(uint8_t x)
{
    return jl_box_uint8(x);
}

inline jl_value_t*
box(float x)
{
    return jl_box_float32(x);
}

inline jl_value_t*
box(double x)
{
    return jl_box_float64(x);
}

inline size_t
gc_preserve(jl_value_t* v)
{
    return Ref_keeper::get()->mark(v);
}

inline void
gc_release(size_t id)
{
    Ref_keeper::get()->unmark(id);
}

inline jl_value_t*
jl_call_func(jl_function_t* f, jl_value_t** args, size_t n, bool jl_safe)
{
    if (jl_safe)
        throw unimplemented_error();
    else
        return Ref_keeper::get()->call(f, args, n);
}

inline jl_value_t*
jl_call_func(std::vector<jl_value_t*>& args, bool jl_safe)
{
    return jl_call_func((jl_function_t*)args[0], &args.data()[1], args.size() - 1, jl_safe);
}

}
}

#endif /* SPU_JULIA */
