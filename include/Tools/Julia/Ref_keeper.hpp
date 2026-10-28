#ifndef REF_KEEPER_HPP_
#define REF_KEEPER_HPP_

#ifdef SPU_JULIA

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
class Ref_keeper
{
  private:
    jl_value_t* last_result;

    size_t size;
    jl_value_t** marks;
    size_t cpt;

    std::vector<size_t> freelist;

    Ref_keeper()
      : last_result(NULL)
      , size(0)
      , marks(NULL)
      , cpt(0)
      , freelist(0)
    {
    }

  public:
    ~Ref_keeper();

    jl_value_t* get_res() const;

    jl_value_t* call0(jl_function_t* f);
    jl_value_t* call1(jl_function_t* f, jl_value_t* a);
    jl_value_t* call(jl_function_t* f, jl_value_t** args, size_t n);
    jl_value_t* safe_call(jl_function_t* f, jl_value_t** args, size_t n);

    jl_value_t* eval(std::string code);
    jl_value_t* unsafe_eval(std::string code);

    size_t mark(jl_value_t* v);
    void unmark(size_t id);
    void update_value(size_t id, jl_value_t* v);

    static void init(size_t size);
    static Ref_keeper* get();
};
}
}

#endif /* SPU_JULIA */
#endif /* REF_KEEPER */
