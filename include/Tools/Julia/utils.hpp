#ifndef JULIA_UTILS_H_
#define JULIA_UTILS_H_

#ifdef SPU_JULIA

#include <julia.h>

namespace spu
{
namespace tools
{
jl_value_t*
jl_eval_file(const char* filename);

}
}

#endif /* SPU_JULIA */
#endif /* JULIA_UTILS_H_ */
