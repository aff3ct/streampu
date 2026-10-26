#ifdef SPU_JULIA

#include <cstdint>
#include <fstream>
#include <julia.h>
#include <sstream>
#include <sys/types.h>

#include "Tools/Exception/unimplemented_error/unimplemented_error.hpp"
#include "Tools/Julia/Ref_keeper.hpp"
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
    end
    ref[] = (res, exception_occurred, exception, backtrace)
    return ref[]
  end
end
)""";

jl_module_t* module;
jl_function_t* assign;
jl_function_t* jl_eval;
jl_function_t* jl_get;
jl_function_t* jl_safe_call;
jl_function_t* mk_ref;
jl_function_t* mycall;

jl_value_t*
jl_eval_file(const char* filename)
{
    std::ifstream is(filename);
    if (!is.is_open())
    {
        std::string alt = (filename[0] == '.' && filename[1] == '.' && filename[2] == '/')
                            ? std::string(filename + 3)
                            : std::string("../") + filename;
        is.open(alt);
        if (!is.is_open())
        {
            std::stringstream message;
            message << "Cannot open file '" << filename << "'.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
    }
    std::stringstream ss;
    (ss << is.rdbuf());
    return jl_eval_string(ss.str().c_str());
}

void
init(uint64_t n_proxies)
{
    (void)jl_eval_string(code);
    module = (jl_module_t*)jl_eval_string("return Test");
    assign = jl_get_function(module, "assign");
    jl_eval = jl_get_function(module, "myeval");
    jl_get = jl_get_function(module, "unref");
    jl_safe_call = jl_get_function(module, "safe_call");
    mk_ref = jl_get_function(module, "mk_ref");
    mycall = jl_get_function(module, "call");
    Ref_keeper::init(n_proxies);
}

//

jl_value_t*
box(int64_t x)
{
    return jl_box_int64(x);
}

jl_value_t*
box(int32_t x)
{
    return jl_box_int32(x);
}

jl_value_t*
box(int16_t x)
{
    return jl_box_int16(x);
}

jl_value_t*
box(int8_t x)
{
    return jl_box_int8(x);
}

jl_value_t*
box(uint64_t x)
{
    return jl_box_uint64(x);
}

jl_value_t*
box(uint32_t x)
{
    return jl_box_uint32(x);
}

jl_value_t*
box(uint16_t x)
{
    return jl_box_uint16(x);
}

jl_value_t*
box(uint8_t x)
{
    return jl_box_uint8(x);
}

jl_value_t*
box(float x)
{
    return jl_box_float32(x);
}

jl_value_t*
box(double x)
{
    return jl_box_float64(x);
}

size_t
gc_preserve(jl_value_t* v)
{
    return Ref_keeper::get()->mark(v);
}

void
gc_release(size_t id)
{
    Ref_keeper::get()->unmark(id);
}

jl_value_t*
jl_call_func(jl_function_t* f, jl_value_t** args, size_t n, bool jl_safe)
{
    if (jl_safe)
        return Ref_keeper::get()->safe_call(f, args, n);
    else
        return Ref_keeper::get()->call(f, args, n);
}

jl_value_t*
jl_call_func(std::vector<jl_value_t*>& args, bool jl_safe)
{
    return jl_call_func((jl_function_t*)args[0], &args.data()[1], args.size() - 1, jl_safe);
}

jl_value_t*
jl_call_func(std::vector<void*>& args, bool jl_safe)
{
    return jl_call_func((jl_function_t*)args[0], &((jl_value_t**)args.data())[1], args.size() - 1, jl_safe);
}
}
}

#endif /* SPU_JULIA */
