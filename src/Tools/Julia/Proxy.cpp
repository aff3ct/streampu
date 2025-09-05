#ifdef SPU_JULIA

#include <julia.h>

#include "Tools/Julia/Proxy.hpp"
#include "Tools/Julia/Ref_keeper.hpp"

namespace spu
{
namespace tools
{
Proxy::Proxy(jl_value_t* value)
  : value(value)
{
    id = Ref_keeper::get()->mark(this->value);
}

Proxy::Proxy(const Proxy& oth)
  : Proxy(oth.value)
{
}

Proxy::~Proxy()
{
    Ref_keeper::get()->unmark(this->id);
}

Proxy::
operator jl_value_t*()
{
    return value;
}

Proxy::
operator const jl_value_t*() const
{
    return value;
}

Proxy::
operator void*()
{
    return (void*)value;
}

Proxy::
operator const void*() const
{
    return (const void*)value;
}

jl_value_t*
Proxy::operator=(jl_value_t* v)
{
    value = v;
    Ref_keeper::get()->update_value(id, v);
    return value;
}

jl_value_t*
Proxy::operator=(const Proxy& oth)
{
    return *this = oth.value;
}

jl_value_t*
Proxy::operator()()
{
    return Ref_keeper::get()->call0(value);
}

jl_value_t*
Proxy::operator()(jl_value_t* a)
{
    return Ref_keeper::get()->call1(value, a);
}

jl_value_t*
Proxy::operator()(const Proxy& a)
{
    return (*this)(a.value);
}

}
}

#endif /* SPU_JULIA */
