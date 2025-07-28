#ifdef SPU_JULIA

#include <fstream>
#include <julia.h>
#include <sstream>

#include "Tools/Julia/utils.hpp"

namespace spu
{
namespace tools
{
jl_value_t*
jl_eval_file(const char* filename)
{
    std::ifstream is(filename);
    std::stringstream ss;
    (ss << is.rdbuf());
    return jl_eval_string(ss.str().c_str());
}

}
}

#endif /* SPU_JULIA */
