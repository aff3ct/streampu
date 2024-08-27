/*!
 * \file
 * \brief Class module::Stateless_Jluna.
 */
#ifndef STATELESS_JLUNA_HPP_
#define STATELESS_JLUNA_HPP_

#ifdef SPU_JLUNA

#include <cstdint>
#include <functional>
#include <jluna.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Module/Module.hpp"
#include "Runtime/Task/Task.hpp"
#include "Tools/Interface/Interface_clone.hpp"

namespace spu
{
namespace module
{
class Stateless_Jluna final
  : public Module
  , public tools::Interface_reset
{
  private:
    std::vector<std::vector<jluna::unsafe::Value*>> jl_constants_ptr;
    std::vector<std::vector<size_t>> jl_constants_id;
    std::vector<std::vector<void*>> jl_func_args;
    bool evaluated;
    std::shared_ptr<size_t> n_clones;

    std::shared_ptr<std::vector<std::vector<std::function<void(Stateless_Jluna& m)>>>> jl_create_constants;
    std::shared_ptr<std::vector<std::function<void()>>> jl_evaluate;
    std::shared_ptr<std::vector<std::function<void(Stateless_Jluna& m)>>> jl_create_codelet;

    bool jl_safe;

  public:
    Stateless_Jluna(const bool jl_safe = true);
    virtual ~Stateless_Jluna();
    virtual Stateless_Jluna* clone() const;
    void deep_copy(const Stateless_Jluna& m);
    virtual void reset();
    bool is_eval() const;

    void set_jl_safety(const bool jl_safe);
    bool is_jl_safe();

    using Module::set_name;
    using Module::set_short_name;

    runtime::Task& create_task(const std::string& name);
    runtime::Task& create_tsk(const std::string& name);

    using Module::create_sck_fwd;
    using Module::create_sck_in;
    using Module::create_sck_out;
    using Module::create_socket_fwd;
    using Module::create_socket_in;
    using Module::create_socket_out;

    // using Module::create_2d_sck_fwd;
    // using Module::create_2d_sck_in;
    // using Module::create_2d_sck_out;
    // using Module::create_2d_socket_fwd;
    // using Module::create_2d_socket_in;
    // using Module::create_2d_socket_out;

    using Module::register_timer;
    using Module::set_n_frames_per_wave;
    using Module::set_single_wave;

    template<typename T>
    void create_constant(runtime::Task& task, const T& value);

    void create_cdl(runtime::Task& task, const std::string& julia_code);
    void create_codelet(runtime::Task& task, const std::string& julia_code);

    void create_cdl_file(runtime::Task& task, const std::string& julia_filepath);
    void create_codelet_file(runtime::Task& task, const std::string& julia_filepath);

    void eval();

  private:
    void _create_codelet(runtime::Task& task);

    static size_t get_task_id(runtime::Task& task);
    static int32_t jl_call_func(const std::vector<void*>& args, const bool jl_safe);
    static jluna::unsafe::Array* jl_new_array_from_data(const runtime::Socket* sck);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Stateless/Stateless_Jluna.hxx"
#endif

#endif /* SPU_JLUNA */

#endif /* STATELESS_JLUNA_HPP_ */
