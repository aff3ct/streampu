/*!
 * \file
 * \brief Class module::Stateful.
 */
#ifndef STATEFUL_HPP_
#define STATEFUL_HPP_

#include <cstdint>

#include "Module/Module.hpp"
#include "Runtime/Task/Task.hpp"

namespace spu
{
namespace module
{

class Stateful : public Module
{
  public:
    Stateful();
    virtual ~Stateful() = default;

  protected:
    using module::Module::deep_copy;

    using module::Module::set_name;
    using module::Module::set_short_name;

    using module::Module::create_task;
    using module::Module::create_tsk;

    using module::Module::create_sck_fwd;
    using module::Module::create_sck_in;
    using module::Module::create_sck_out;
    using module::Module::create_socket_fwd;
    using module::Module::create_socket_in;
    using module::Module::create_socket_out;

    using module::Module::create_2d_sck_fwd;
    using module::Module::create_2d_sck_in;
    using module::Module::create_2d_sck_out;
    using module::Module::create_2d_socket_fwd;
    using module::Module::create_2d_socket_in;
    using module::Module::create_2d_socket_out;

    using module::Module::create_cdl;
    using module::Module::create_codelet;
    using module::Module::register_timer;
    using module::Module::set_n_frames_per_wave;
    using module::Module::set_single_wave;
};
}
}

#endif /* STATEFUL_HPP_ */
