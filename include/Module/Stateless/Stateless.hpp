/*!
 * \file
 * \brief Class module::Stateless.
 */
#ifndef STATELESS_HPP_
#define STATELESS_HPP_

#include <cstdint>

#include "Module/Module.hpp"

namespace spu
{
namespace module
{

class Stateless final : public Module
{
  public:
    Stateless();
    virtual ~Stateless() = default;
    virtual Stateless* clone() const;

    using Module::set_name;
    using Module::set_short_name;

    using Module::create_task;
    using Module::create_tsk;

    using Module::create_sck_fwd;
    using Module::create_sck_in;
    using Module::create_sck_out;
    using Module::create_socket_fwd;
    using Module::create_socket_in;
    using Module::create_socket_out;

    using Module::create_2d_sck_fwd;
    using Module::create_2d_sck_in;
    using Module::create_2d_sck_out;
    using Module::create_2d_socket_fwd;
    using Module::create_2d_socket_in;
    using Module::create_2d_socket_out;

    using Module::create_cdl;
    using Module::create_codelet;
    using Module::register_timer;
    using Module::set_n_frames_per_wave;
    using Module::set_single_wave;
};
}
}

#endif /* STATELESS_HPP_ */
