#ifdef SPU_JULIA

#include <csignal>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

#include "Tools/Exception/runtime_error/runtime_error.hpp"
#include "Tools/Julia/Julia.hpp"
#include "Tools/Julia/Ref_keeper.hpp"

namespace spu
{
namespace tools
{

static const char* julia_bootstrap_code = R"""(
module StreamPU
  # Create a boxed mutable reference cell initialized to nothing.
  # This cell is rooted in the thread's GC frame (Ref_keeper) to safely hold
  # temporary values and return results across C++/Julia boundaries.
  function mk_ref()
    return Ref{Any}(nothing)
  end

  # Assign a value into an existing reference cell.
  # Ensures the assigned value is immediately rooted and protected from GC.
  function assign(ref::Ref{Any}, v)
    ref[] = v
  end

  # Dereference a reference cell to retrieve the underlying stored value.
  function unref(ref::Ref{Any})
    return ref[]
  end

  # Call a Julia function and immediately store the returned value into ref[]
  # before returning control to C++.
  # This guarantees that the result is rooted in the GC frame before any subsequent
  # GC sweep can collect it.
  function call(ref::Ref{Any}, f::Function, args...)
    ref[] = f(args...)
    return ref[]
  end

  # Evaluate a string representation of Julia code (passed as Char array).
  # Used by Ref_keeper::eval() to parse and execute dynamic code expressions.
  function myeval(array::Array{Char, 1})
    str = String(array)
    return eval(Meta.parse(str))
  end

  # Safe invocation of Julia functions with in-engine exception handling.
  # Native C API calls (jl_call) do not automatically propagate Julia exceptions to C++.
  # safe_call executes f(args...) in a try/catch block, formats the full backtrace
  # on failure, and stores the tuple (result, exception_occurred, exception, backtrace)
  # into ref[]. C++ inspects this tuple and throws a spu::tools::runtime_error if needed.
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

jl_module_t* Julia::module = nullptr;
jl_function_t* Julia::assign_func = nullptr;
jl_function_t* Julia::eval_func = nullptr;
jl_function_t* Julia::unref_func = nullptr;
jl_function_t* Julia::safe_call_func = nullptr;
jl_function_t* Julia::mk_ref_func = nullptr;
jl_function_t* Julia::call_func_handle = nullptr;
bool Julia::is_initialized = false;
size_t Julia::n_proxies = 256;

static std::mutex julia_eval_mutex;

void
Julia::init(const uint64_t n_proxies)
{
    if (Julia::is_initialized) return;

    Julia::n_proxies = n_proxies;

    // Preserve any pre-existing SIGINT handler (e.g. spu::tools::Signal_handler) across jl_init(),
    // which otherwise overwrites SIGINT with Julia's internal signal handler.
#if defined(_WIN64) || defined(_WIN32)
    void (*prev_sigint)(int) = SIG_DFL;
    const bool restore_sigint = !jl_is_initialized();
    if (restore_sigint)
        prev_sigint = signal(SIGINT, SIG_DFL);
#else
    struct sigaction prev_sigint = {};
    const bool restore_sigint = !jl_is_initialized() && (sigaction(SIGINT, nullptr, &prev_sigint) == 0);
#endif

    if (!jl_is_initialized())
    {
        jl_init();
    }

    std::lock_guard<std::mutex> lock(julia_eval_mutex);
    (void)jl_eval_string(julia_bootstrap_code);
    Julia::module = (jl_module_t*)jl_eval_string("return StreamPU");
    Julia::assign_func = jl_get_function(Julia::module, "assign");
    Julia::eval_func = jl_get_function(Julia::module, "myeval");
    Julia::unref_func = jl_get_function(Julia::module, "unref");
    Julia::safe_call_func = jl_get_function(Julia::module, "safe_call");
    Julia::mk_ref_func = jl_get_function(Julia::module, "mk_ref");
    Julia::call_func_handle = jl_get_function(Julia::module, "call");

    Ref_keeper::init(n_proxies);

#if defined(_WIN64) || defined(_WIN32)
    if (restore_sigint && prev_sigint != SIG_ERR && prev_sigint != SIG_DFL && prev_sigint != SIG_IGN)
        signal(SIGINT, prev_sigint);
#else
    if (restore_sigint && prev_sigint.sa_handler != SIG_DFL && prev_sigint.sa_handler != SIG_IGN)
        sigaction(SIGINT, &prev_sigint, nullptr);
#endif

    Julia::is_initialized = true;
}

void
Julia::exit(const int status)
{
    if (jl_is_initialized())
    {
        jl_atexit_hook(status);
        Julia::is_initialized = false;
        Julia::module = nullptr;
    }
}

bool
Julia::is_init()
{
    return Julia::is_initialized;
}

size_t
Julia::get_n_proxies()
{
    return Julia::n_proxies;
}

jl_module_t*
Julia::get_module()
{
    return Julia::module;
}

jl_function_t*
Julia::get_assign()
{
    return Julia::assign_func;
}

jl_function_t*
Julia::get_eval()
{
    return Julia::eval_func;
}

jl_function_t*
Julia::get_unref()
{
    return Julia::unref_func;
}

jl_value_t*
Julia::unref(jl_value_t* ref)
{
    return jl_call1(Julia::unref_func, ref);
}

jl_function_t*
Julia::get_safe_call()
{
    return Julia::safe_call_func;
}

jl_function_t*
Julia::get_mk_ref()
{
    return Julia::mk_ref_func;
}

jl_function_t*
Julia::get_call()
{
    return Julia::call_func_handle;
}

jl_value_t*
Julia::eval_string(const std::string& code)
{
    std::lock_guard<std::mutex> lock(julia_eval_mutex);
    return jl_eval_string(code.c_str());
}

jl_value_t*
Julia::eval_file(const std::string& filename)
{
    std::ifstream is(filename);
    if (!is.is_open())
    {
        std::string alt = (filename.size() >= 3 && filename[0] == '.' && filename[1] == '.' && filename[2] == '/')
                            ? filename.substr(3)
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
    ss << is.rdbuf();
    return Julia::eval_string(ss.str());
}

jl_value_t*
Julia::box(const int64_t x)
{
    return jl_box_int64(x);
}

jl_value_t*
Julia::box(const int32_t x)
{
    return jl_box_int32(x);
}

jl_value_t*
Julia::box(const int16_t x)
{
    return jl_box_int16(x);
}

jl_value_t*
Julia::box(const int8_t x)
{
    return jl_box_int8(x);
}

jl_value_t*
Julia::box(const uint64_t x)
{
    return jl_box_uint64(x);
}

jl_value_t*
Julia::box(const uint32_t x)
{
    return jl_box_uint32(x);
}

jl_value_t*
Julia::box(const uint16_t x)
{
    return jl_box_uint16(x);
}

jl_value_t*
Julia::box(const uint8_t x)
{
    return jl_box_uint8(x);
}

jl_value_t*
Julia::box(const float x)
{
    return jl_box_float32(x);
}

jl_value_t*
Julia::box(const double x)
{
    return jl_box_float64(x);
}

jl_value_t*
Julia::box(const bool x)
{
    return jl_box_bool(x);
}

size_t
Julia::preserve(jl_value_t* v)
{
    return Ref_keeper::get()->mark(v);
}

void
Julia::release(const size_t id)
{
    Ref_keeper::get()->unmark(id);
}

jl_value_t*
Julia::call(jl_function_t* f, jl_value_t** args, const size_t n, const bool safe)
{
    if (safe)
        return Ref_keeper::get()->safe_call(f, args, n);
    else
        return Ref_keeper::get()->call(f, args, n);
}

jl_value_t*
Julia::call(std::vector<jl_value_t*>& args, const bool safe)
{
    return Julia::call((jl_function_t*)args[0], &args.data()[1], args.size() - 1, safe);
}

jl_value_t*
Julia::call(std::vector<void*>& args, const bool safe)
{
    return Julia::call((jl_function_t*)args[0], (jl_value_t**)&args.data()[1], args.size() - 1, safe);
}

}
}

#endif /* SPU_JULIA */
