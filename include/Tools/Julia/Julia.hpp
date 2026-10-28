/*!
 * \file
 * \brief Class tools::Julia.
 */
#ifndef TOOLS_JULIA_HPP_
#define TOOLS_JULIA_HPP_

#ifdef SPU_JULIA

#include <cstddef>
#include <cstdint>
#include <julia.h>
#include <string>
#include <vector>

#if JULIA_VERSION_MAJOR > 1 || (JULIA_VERSION_MAJOR == 1 && JULIA_VERSION_MINOR >= 13)
typedef jl_value_t jl_function_t;
#endif

namespace spu
{
namespace tools
{

/*!
 * \class Julia
 *
 * \brief Static utility class providing a high-level encapsulation of the native Julia C runtime (`libjulia`).
 *
 * This class handles Julia runtime lifecycle management, thread-safe dynamic code evaluation,
 * type boxing for C++ primitive types, GC rooting via the per-thread Ref_keeper mechanism,
 * and protected function invocation with Julia exception translation.
 */
class Julia
{
  private:
    static jl_module_t* module;
    static jl_function_t* assign_func;
    static jl_function_t* eval_func;
    static jl_function_t* unref_func;
    static jl_function_t* safe_call_func;
    static jl_function_t* mk_ref_func;
    static jl_function_t* call_func_handle;
    static bool is_initialized;
    static size_t n_proxies;

  public:
    // =========================================================================
    // Lifecycle management
    // =========================================================================

    /*!
     * \brief Initializes the Julia embedded runtime and bootstraps the internal `StreamPU` Julia module.
     *
     * If the Julia engine is not already started, it calls `jl_init()`. It then compiles internal
     * helper functions for GC rooting and exception handling, and initializes the calling thread's Ref_keeper.
     *
     * \param n_proxies Maximum number of simultaneously preserved GC references per thread (defaults to 256).
     */
    static void init(const uint64_t n_proxies = 256);

    /*!
     * \brief Shuts down the Julia runtime and invokes all registered exit hooks.
     *
     * \param status Exit status code passed to `jl_atexit_hook()` (defaults to 0).
     */
    static void exit(const int status = 0);

    /*!
     * \brief Checks whether the Julia runtime has been initialized via `Julia::init()`.
     *
     * \return `true` if initialized, `false` otherwise.
     */
    static bool is_init();

    /*!
     * \brief Returns the maximum number of GC references tracked per thread configured at initialization.
     *
     * \return The configured capacity (`n_proxies`).
     */
    static size_t get_n_proxies();

    // =========================================================================
    // Module & internal function handles
    // =========================================================================

    /*!
     * \brief Returns the internal `StreamPU` Julia module pointer.
     *
     * \return Pointer to the `StreamPU` module (`jl_module_t*`).
     */
    static jl_module_t* get_module();

    /*!
     * \brief Returns the internal `StreamPU.assign` function handle.
     *
     * \return Pointer to the `assign` function (`jl_function_t*`).
     */
    static jl_function_t* get_assign();

    /*!
     * \brief Returns the internal `StreamPU.myeval` function handle.
     *
     * \return Pointer to the `myeval` function (`jl_function_t*`).
     */
    static jl_function_t* get_eval();

    /*!
     * \brief Returns the internal `StreamPU.unref` function handle.
     *
     * \return Pointer to the `unref` function (`jl_function_t*`).
     */
    static jl_function_t* get_unref();

    /*!
     * \brief Returns the internal `StreamPU.safe_call` function handle.
     *
     * \return Pointer to the `safe_call` function (`jl_function_t*`).
     */
    static jl_function_t* get_safe_call();

    /*!
     * \brief Returns the internal `StreamPU.mk_ref` function handle.
     *
     * \return Pointer to the `mk_ref` function (`jl_function_t*`).
     */
    static jl_function_t* get_mk_ref();

    /*!
     * \brief Returns the internal `StreamPU.call` function handle.
     *
     * \return Pointer to the `call` function (`jl_function_t*`).
     */
    static jl_function_t* get_call();

    // =========================================================================
    // Value manipulation & GC preservation helpers
    // =========================================================================

    /*!
     * \brief Extracts and returns the underlying value stored inside a Julia `Ref{Any}` cell.
     *
     * \param ref Pointer to the `Ref{Any}` Julia object.
     * \return Pointer to the unwrapped value (`jl_value_t*`).
     */
    static jl_value_t* unref(jl_value_t* ref);

    /*!
     * \brief Preserves a Julia object from Garbage Collector sweeps by rooting it in the current thread's frame.
     *
     * \param v The Julia value to protect.
     * \return An identifier token (`size_t`) used to unroot/release the reference later.
     */
    static size_t preserve(jl_value_t* v);

    /*!
     * \brief Releases a previously rooted Julia reference from the current thread's GC frame.
     *
     * \param id The identifier token returned by `Julia::preserve()`.
     */
    static void release(const size_t id);

    // =========================================================================
    // Dynamic code evaluation
    // =========================================================================

    /*!
     * \brief Evaluates a string of Julia code in the global scope.
     *
     * \param code String containing valid Julia expressions.
     * \return The evaluated Julia object (`jl_value_t*`).
     * \throw spu::tools::runtime_error If parsing or evaluation raises a Julia exception.
     */
    static jl_value_t* eval_string(const std::string& code);

    /*!
     * \brief Reads a file from disk and evaluates its contents in the global Julia scope.
     *
     * \param filename Absolute or relative filesystem path to the Julia script.
     * \return The evaluated Julia object (`jl_value_t*`).
     * \throw spu::tools::runtime_error If the file cannot be opened or if evaluation fails.
     */
    static jl_value_t* eval_file(const std::string& filename);

    // =========================================================================
    // Type boxing (primitive C++ types -> Julia heap values)
    // =========================================================================

    /*!
     * \brief Boxes a 64-bit signed integer into a Julia `Int64` object.
     * \param x Value to box.
     * \return Pointer to the boxed Julia value (`jl_value_t*`).
     */
    static jl_value_t* box(const int64_t x);

    /*!
     * \brief Boxes a 32-bit signed integer into a Julia `Int32` object.
     * \param x Value to box.
     * \return Pointer to the boxed Julia value (`jl_value_t*`).
     */
    static jl_value_t* box(const int32_t x);

    /*!
     * \brief Boxes a 16-bit signed integer into a Julia `Int16` object.
     * \param x Value to box.
     * \return Pointer to the boxed Julia value (`jl_value_t*`).
     */
    static jl_value_t* box(const int16_t x);

    /*!
     * \brief Boxes an 8-bit signed integer into a Julia `Int8` object.
     * \param x Value to box.
     * \return Pointer to the boxed Julia value (`jl_value_t*`).
     */
    static jl_value_t* box(const int8_t x);

    /*!
     * \brief Boxes a 64-bit unsigned integer into a Julia `UInt64` object.
     * \param x Value to box.
     * \return Pointer to the boxed Julia value (`jl_value_t*`).
     */
    static jl_value_t* box(const uint64_t x);

    /*!
     * \brief Boxes a 32-bit unsigned integer into a Julia `UInt32` object.
     * \param x Value to box.
     * \return Pointer to the boxed Julia value (`jl_value_t*`).
     */
    static jl_value_t* box(const uint32_t x);

    /*!
     * \brief Boxes a 16-bit unsigned integer into a Julia `UInt16` object.
     * \param x Value to box.
     * \return Pointer to the boxed Julia value (`jl_value_t*`).
     */
    static jl_value_t* box(const uint16_t x);

    /*!
     * \brief Boxes an 8-bit unsigned integer into a Julia `UInt8` object.
     * \param x Value to box.
     * \return Pointer to the boxed Julia value (`jl_value_t*`).
     */
    static jl_value_t* box(const uint8_t x);

    /*!
     * \brief Boxes a single-precision floating point number into a Julia `Float32` object.
     * \param x Value to box.
     * \return Pointer to the boxed Julia value (`jl_value_t*`).
     */
    static jl_value_t* box(const float x);

    /*!
     * \brief Boxes a double-precision floating point number into a Julia `Float64` object.
     * \param x Value to box.
     * \return Pointer to the boxed Julia value (`jl_value_t*`).
     */
    static jl_value_t* box(const double x);

    /*!
     * \brief Boxes a boolean into a Julia `Bool` object.
     * \param x Value to box.
     * \return Pointer to the boxed Julia value (`jl_value_t*`).
     */
    static jl_value_t* box(const bool x);

    // =========================================================================
    // Function execution wrappers
    // =========================================================================

    /*!
     * \brief Invokes a Julia function with arguments, ensuring the returned object is immediately rooted.
     *
     * \param f Pointer to the target Julia function (`jl_function_t*`).
     * \param args Array of pointers to Julia value arguments (`jl_value_t**`).
     * \param n Number of arguments in `args`.
     * \param safe If `true`, intercepts any Julia exception and throws a `spu::tools::runtime_error`
     *             with the full Julia backtrace. If `false`, calls Julia directly without try/catch overhead.
     * \return The result of the function execution (`jl_value_t*`), rooted in the current thread's frame.
     */
    static jl_value_t* call(jl_function_t* f, jl_value_t** args, const size_t n, const bool safe = false);

    /*!
     * \brief Invokes a Julia function represented as the first element of an argument vector.
     *
     * \param args Vector where `args[0]` is the function pointer and `args[1..n-1]` are its arguments.
     * \param safe If `true`, catches Julia exceptions and converts them into C++ exceptions.
     * \return The result of the function execution (`jl_value_t*`).
     */
    static jl_value_t* call(std::vector<jl_value_t*>& args, const bool safe = false);

    /*!
     * \brief Convenience overload invoking a function from an array of untyped `void*` pointers.
     *
     * \param args Vector where `args[0]` is `reinterpret_cast<jl_function_t*>` and subsequent elements
     *             are `reinterpret_cast<jl_value_t*>`.
     * \param safe If `true`, catches Julia exceptions and converts them into C++ exceptions.
     * \return The result of the function execution (`jl_value_t*`).
     */
    static jl_value_t* call(std::vector<void*>& args, const bool safe = false);
};

}
}

#endif /* SPU_JULIA */

#endif /* TOOLS_JULIA_HPP_ */
