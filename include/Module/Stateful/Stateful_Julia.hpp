/*!
 * \file
 * \brief Class module::Stateful_Julia.
 */
#ifndef STATEFUL_JULIA_HPP_
#define STATEFUL_JULIA_HPP_

#ifdef SPU_JULIA

#include <julia.h>
#include <string>
#include <vector>

#include "Module/Module_Julia.hpp"
#include "Module/Stateful/Stateful.hpp"
#include "Runtime/Task/Task.hpp"

namespace spu
{
namespace module
{
class Stateful_Julia
  : public Stateful
  , public Module_Julia
{
  public:
    Stateful_Julia(const bool jl_safe = true);
    virtual ~Stateful_Julia() = default;
    virtual Stateful_Julia* clone() const;
    void deep_copy(const Stateful_Julia& m);

    using Stateful::set_name;
    using Stateful::set_short_name;

    runtime::Task& create_task(const std::string& name);
    runtime::Task& create_tsk(const std::string& name);

    using Stateful::create_sck_fwd;
    using Stateful::create_sck_in;
    using Stateful::create_sck_out;
    using Stateful::create_socket_fwd;
    using Stateful::create_socket_in;
    using Stateful::create_socket_out;

    using Stateful::create_2d_sck_fwd;
    using Stateful::create_2d_sck_in;
    using Stateful::create_2d_sck_out;
    using Stateful::create_2d_socket_fwd;
    using Stateful::create_2d_socket_in;
    using Stateful::create_2d_socket_out;

    using Stateful::register_timer;
    using Stateful::set_n_frames_per_wave;
    using Stateful::set_single_wave;

    using Module_Julia::create_cdl;
    using Module_Julia::create_cdl_file;
    using Module_Julia::create_cdl_function;
    using Module_Julia::create_codelet;
    using Module_Julia::create_codelet_file;
    using Module_Julia::create_codelet_function;

    void create_state(const std::string& julia_constructor,
                      const bool shared = false,
                      const std::string& julia_code = "");
    void create_stt(const std::string& julia_constructor,
                    const bool shared = false,
                    const std::string& julia_code = "");

    void create_state_file(const std::string& julia_constructor,
                           const bool shared = false,
                           const std::string& julia_filepath = "");
    void create_stt_file(const std::string& julia_constructor,
                         const bool shared = false,
                         const std::string& julia_filepath = "");

    const std::vector<jl_value_t*>& get_states() const;
};
}
}

#endif /* SPU_JULIA */

#endif /* STATEFUL_JULIA_HPP_ */
