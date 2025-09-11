#ifndef JULIA_UTILS_H_
#define JULIA_UTILS_H_

#ifdef SPU_JULIA

#include <julia.h>
#include <vector>

namespace spu
{
namespace tools
{
extern jl_module_t* module;
extern jl_function_t* assign;
extern jl_function_t* jl_eval;
extern jl_function_t* jl_get;
extern jl_function_t* jl_safe_call;
extern jl_function_t* mk_ref;
extern jl_function_t* mycall;

jl_value_t*
jl_eval_file(const char* filename);

void
init(uint64_t n_proxies);

jl_value_t*
box(int64_t x);
jl_value_t*
box(int32_t x);
jl_value_t*
box(int16_t x);
jl_value_t*
box(int8_t x);
jl_value_t*
box(uint64_t x);
jl_value_t*
box(uint32_t x);
jl_value_t*
box(uint16_t x);
jl_value_t*
box(uint8_t x);

size_t
gc_preserve(jl_value_t* v);
void
gc_release(size_t id);

jl_value_t*
jl_call_func(jl_function_t* f, jl_value_t** args, size_t n, bool jl_safe);
jl_value_t*
jl_call_func(std::vector<jl_value_t*>& args, bool jl_safe);
jl_value_t*
jl_call_func(std::vector<void*>& args, bool jl_safe);
}
}

#endif /* SPU_JULIA */
#endif /* JULIA_UTILS_H_ */
