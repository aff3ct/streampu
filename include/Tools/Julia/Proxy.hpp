#ifndef PROXY_HPP_
#define PROXY_HPP_

#ifdef SPU_JULIA

#include <initializer_list>
#include <julia.h>
#include <vector>

#include "Tools/Julia/Ref_keeper.hpp"

namespace spu
{
namespace tools
{
class Proxy
{
  private:
    jl_value_t* value;
    size_t id; // For Ref_keeper

  public:
    Proxy(jl_value_t* value = NULL);
    Proxy(const Proxy& oth);
    ~Proxy();

    operator jl_value_t*() const;
    operator const jl_value_t*() const;
    operator void*();
    operator const void*() const;

    jl_value_t* operator=(jl_value_t* v);
    jl_value_t* operator=(const Proxy& oth);
    jl_value_t* operator()() const;
    jl_value_t* operator()(jl_value_t* a) const;
    jl_value_t* operator()(const Proxy& a) const;

    // TODO: Remove generic typename for jl_value_t
    template<typename... Arg_t>
    jl_value_t* operator()(const Arg_t&... args) const
    {
        jl_value_t* array[] = { (jl_value_t*)args... };
        return Ref_keeper::get()->call(value, array, sizeof...(args));
    }
};
}
}

#endif /* SPU_JULIA */
#endif /* PROXY_HPP_ */
