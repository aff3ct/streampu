#include <chrono>
#include <sstream>
#include <string>
#include <thread>

#include "Module/Stateful/Iterator/Iterator.hpp"

using namespace spu;
using namespace spu::module;

Iterator::Iterator(const size_t limit)
  : Stateful()
  , limit(limit)
  , counter(0)
{
    const std::string name = "Iterator";
    this->set_name(name);
    this->set_short_name(name);
    this->set_single_wave(true);

    auto& p1 = this->create_task("iterate");
    const auto p1s_out = this->create_socket_out(p1, "out", 1, typeid(int8_t));
    this->create_codelet(p1,
                         [p1s_out](Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& ite = static_cast<Iterator&>(m);
                             ite._iterate(t[p1s_out].get_dataptr<int8_t>(), frame_id);
                             return runtime::status_t::SUCCESS;
                         });

    auto& p2 = this->create_task("reset");
    this->create_codelet(p2,
                         [](Module& m, runtime::Task& /*t*/, const size_t /*frame_id*/) -> int
                         {
                             auto& ite = static_cast<Iterator&>(m);
                             ite.reset();
                             return runtime::status_t::SUCCESS;
                         });
}

Iterator*
Iterator::clone() const
{
    auto m = new Iterator(*this);
    m->deep_copy(*this);
    return m;
}

size_t
Iterator::get_limit() const
{
    return this->limit;
}

void
Iterator::set_limit(const size_t limit)
{
    this->limit = limit;
    this->counter = this->counter % (this->limit + 1);
}

void
Iterator::reset()
{
    this->counter = 0;
}

void
Iterator::iterate(int8_t* out, const int frame_id, const bool managed_memory)
{
    (*this)[ite::sck::iterate::out].bind(out);
    (*this)[ite::tsk::iterate].exec(frame_id, managed_memory);
}

void
Iterator::_iterate(int8_t* out, const size_t /*frame_id*/)
{
    size_t is_over_path = this->counter < this->limit ? 0 : 1;
    this->counter++;
    this->counter = this->counter % (this->limit + 1);
    out[0] = is_over_path;
}
