#include <string>

#include "Module/Stateful/Controller/Controller.hpp"

using namespace spu;
using namespace spu::module;

Controller::Controller(const size_t init_path)
  : Stateful()
  , init_path(init_path)
  , path(init_path)
{
    const std::string name = "Controller";
    this->set_short_name(name);
    this->set_single_wave(true);

    auto& p1 = this->create_task("control");
    const auto p1s_out = this->create_socket_out(p1, "out", 1, typeid(int8_t));
    this->create_codelet(p1,
                         [p1s_out](Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& ctr = static_cast<Controller&>(m);
                             ctr._control(t[p1s_out].get_dataptr<int8_t>(), frame_id);
                             return runtime::status_t::SUCCESS;
                         });

    auto& p2 = this->create_task("reset");
    this->create_codelet(p2,
                         [](Module& m, runtime::Task& /*t*/, const size_t /*frame_id*/) -> int
                         {
                             auto& ctr = static_cast<Controller&>(m);
                             ctr.reset();
                             return runtime::status_t::SUCCESS;
                         });
}

void
Controller::set_path(const size_t path)
{
    this->path = path;
}

size_t
Controller::get_path() const
{
    return this->path;
}

void
Controller::reset()
{
    this->path = this->init_path;
}

void
Controller::control(int8_t* out, const int frame_id, const bool managed_memory)
{
    (*this)[ctr::sck::control::out].bind(out);
    (*this)[ctr::tsk::control].exec(frame_id, managed_memory);
}
