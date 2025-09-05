#ifdef SPU_JULIA

#include <fstream>
#include <julia.h>
#include <sstream>

#include "Tools/Julia/utils.hpp"

namespace spu
{
namespace tools
{
static const char* code = R"""(
module Test
  function assign(ref::Ref{Any}, v)
    ref[] = v
  end

  function call(ref::Ref{Any}, f::Function, args...)
    ref[] = f(args...)
    return ref[]
  end

  function unref(ref::Ref{Any})
    return ref[]
  end

  function mk_ref()
    return Ref{Any}(nothing)
  end

  function myeval(array::Array{Char, 1})
    str = String(array)
    return eval(Meta.parse(str))
  end

  # Code from jluna
  function safe_call(ref::Ref{Any}, f::Function, args...)
    res::Any = undef
    backtrace::String = ""
    exception_occurred::Bool = false
    exception::Union{Exception, UndefInitializer} = undef
    try
      res = f(args...)
    catch e
      exception = e
      backtrace = sprint(Base.showerror, exception, catch_backtrace())
      exception_occurred = true
      println(exception)
      println(backtrace)
      throw(e)
    end
    return res # TODO: remove
    return (res, exception_occurred, exception, backtrace)
  end
end
)""";

jl_module_t* module;
jl_function_t* assign;
jl_function_t* jl_eval;
jl_function_t* jl_get;
jl_function_t* mk_ref;
jl_function_t* mycall;

jl_value_t*
jl_eval_file(const char* filename)
{
    std::ifstream is(filename);
    std::stringstream ss;
    (ss << is.rdbuf());
    return jl_eval_string(ss.str().c_str());
}

void
init(uint64_t n_threads)
{
    (void)n_threads;
    (void)jl_eval_string(code);
    module = (jl_module_t*)jl_eval_string("return Test");
    assign = jl_get_function(module, "assign");
    jl_eval = jl_get_function(module, "myeval");
    jl_get = jl_get_function(module, "unref");
    mk_ref = jl_get_function(module, "mk_ref");
    mycall = jl_get_function(module, "call");
    // TODO init proxies
}

}
}

#endif /* SPU_JULIA */
