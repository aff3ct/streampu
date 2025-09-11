#include "Tools/Exception/runtime_error/runtime_error.hpp"
#ifdef SPU_JULIA

#include <cstddef>
#include <iostream>
#include <julia.h>

#include "Tools/Julia/Ref_keeper.hpp"
#include "Tools/Julia/utils.hpp"

namespace spu
{
namespace tools
{
Ref_keeper::~Ref_keeper()
{
    delete[] marks;
}

jl_value_t*
Ref_keeper::get_res() const
{
    return jl_call1(jl_get, last_result);
}

jl_value_t*
Ref_keeper::call0(jl_function_t* f)
{
    return jl_call2(mycall, last_result, f);
}

jl_value_t*
Ref_keeper::call1(jl_function_t* f, jl_value_t* a)
{
    return jl_call3(mycall, last_result, f, a);
}

static inline jl_value_t*
_call(jl_function_t* call, jl_value_t* ref, jl_function_t* f, jl_value_t** args, size_t n)
{
    jl_value_t** args2;
    JL_GC_PUSHARGS(args2, n + 2);
    args2[0] = ref;
    args2[1] = f;
    for (size_t i = 0; i < n; i++)
    {
        args2[i + 2] = args[i];
    }
    jl_value_t* res = jl_call(call, args2, n + 2); // NULL for some reason
    JL_GC_POP();
    return res;
}

jl_value_t*
Ref_keeper::call(jl_function_t* f, jl_value_t** args, size_t n)
{
    jl_value_t** args2;
    JL_GC_PUSHARGS(args2, n + 2);
    args2[0] = last_result;
    args2[1] = f;
    for (size_t i = 0; i < n; i++)
    {
        args2[i + 2] = args[i];
    }
    jl_value_t* res = jl_call(mycall, args2, n + 2); // NULL for some reason
    JL_GC_POP();
    return res;
}

jl_value_t*
Ref_keeper::safe_call(jl_function_t* f, jl_value_t** args, size_t n)
{
    jl_value_t* res = _call(jl_safe_call, last_result, f, args, n);
    if (jl_unbox_bool(jl_get_nth_field(res, 1)))
        throw runtime_error(__FILE__, __LINE__, __func__, jl_string_ptr(jl_get_nth_field(res, 3)));
    // throw JuliaException(jl_get_nth_field(res, 2), jl_string_ptr(jl_get_nth_field(res, 3)));
    return jl_get_nth_field(res, 0);
}

jl_value_t*
Ref_keeper::eval(std::string code)
{
    std::cout << "[C] eval" << std::endl;
    size_t len = code.length() + 1;
    char* s = (char*)malloc(sizeof(char) * len);
    size_t i = 0;
    for (char c : code)
    {
        s[i] = c;
        i++;
    }
    // char* test = std::vector<char>(code.begin(), code.end()).data();
    static jl_value_t* array_type = jl_apply_array_type((jl_value_t*)jl_char_type, 1);
    jl_array_t* c = jl_ptr_to_array_1d(array_type, s, len, false);
    // jluna::safe_call(print, c);
    // TODO: jluna::safe_call(jl_eval, c);
    jl_value_t* res = this->call1(jl_eval, (jl_value_t*)c);
    free(s);
    return res;
}

jl_value_t*
Ref_keeper::unsafe_eval(std::string code)
{
    jl_value_t* res = jl_eval_string(code.c_str());
    jl_call2(assign, last_result, res);
    return res;
}

size_t
Ref_keeper::mark(jl_value_t* v)
{
    size_t r;
    if (freelist.empty())
    {
        if (cpt < size + 3)
        {
            r = cpt;
            cpt++;
        }
        else
        {
            std::cout << "Error: no space avaible for marks: cpt=" << cpt << "; size=" << size + 3 << std::endl;
            exit(1); // TODO: throw
        }
    }
    else
    {
        r = freelist.back();
        freelist.pop_back();
    }
    marks[r] = v;
    return r;
}

void
Ref_keeper::update_value(size_t id, jl_value_t* v)
{
    if (id <= 2)
    {
        std::cerr << "Error: trying to change Ref_keeper's protected value" << std::endl;
        exit(1);
    }
    marks[id] = v;
}

void
Ref_keeper::unmark(size_t id)
{
    marks[id] = last_result;
    freelist.push_back(id);
}

void
Ref_keeper::init(size_t size)
{
    Ref_keeper* singleton = get();
    singleton->last_result = jl_call0(mk_ref);
    singleton->size = size;
    singleton->marks = new jl_value_t*[size + 3];
    singleton->cpt = 3; // 0 and 1 for gcframe, 2 for last_result
    singleton->marks[0] = (jl_value_t*)JL_GC_ENCODE_PUSHARGS(size);
    singleton->marks[1] = (jl_value_t*)jl_pgcstack;
    for (size_t i = 0; i < size + 1; i++)
        singleton->marks[i + 2] = singleton->last_result;
    jl_pgcstack = (jl_gcframe_t*)singleton->marks;
}

Ref_keeper*
Ref_keeper::get()
{
    static thread_local Ref_keeper singleton;
    return &singleton;
}

}
}

#endif /* SPU_JULIA */
