#ifndef JULIA_UTILS_H_
#define JULIA_UTILS_H_

#ifdef SPU_JULIA

#include <julia.h>

namespace spu
{
namespace tools
{
extern jl_module_t* module;
extern jl_function_t* assign;
extern jl_function_t* jl_eval;
extern jl_function_t* jl_get;
extern jl_function_t* mk_ref;
extern jl_function_t* mycall;

jl_value_t*
jl_eval_file(const char* filename);

void
init(uint64_t n_threads);
}
}

#endif /* SPU_JULIA */
#endif /* JULIA_UTILS_H_ */
