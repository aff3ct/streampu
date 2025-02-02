#include <cmath>
#include <sstream>
#include <string>

#include "Module/Stateful/Adaptor/Adaptor_m_to_n.hpp"
#include "Tools/Exception/exception.hpp"
#include "Tools/compute_bytes.h"

namespace spu
{
namespace module
{

Adaptor_m_to_n::Adaptor_m_to_n(const std::vector<size_t>& n_elmts,
                               const std::vector<std::type_index>& datatype,
                               const size_t buffer_size,
                               const bool active_waiting)
  : Stateful()
  , n_elmts(n_elmts)
  , n_bytes(tools::compute_bytes(n_elmts, datatype))
  , datatype(datatype)
  , buffer_size(buffer_size)
  , n_sockets(n_elmts.size())
  , buffer(new std::vector<std::vector<std::vector<int8_t*>>>(
      1,
      std::vector<std::vector<int8_t*>>(n_sockets, std::vector<int8_t*>(buffer_size))))
  , first(new std::vector<std::atomic<uint64_t>>(1000))
  , last(new std::vector<std::atomic<uint64_t>>(1000))
  , waiting_canceled(new std::atomic<bool>(false))
  , no_copy_pull(false)
  , no_copy_push(false)
  , active_waiting(active_waiting)
  , cnd_push(new std::vector<std::condition_variable>(1000))
  , mtx_push(new std::vector<std::mutex>(1000))
  , cnd_pull(new std::vector<std::condition_variable>(1000))
  , mtx_pull(new std::vector<std::mutex>(1000))
  , tid_push(0)
  , tid_pull(0)
  , n_pushers(new size_t(1))
  , n_pullers(new size_t(1))
  , n_clones(new size_t(0))
  , buffers_allocated(new bool(false))
  , cloned(false)
  , cur_push_id(0)
  , cur_pull_id(0)
{
    const std::string name = "Adaptor_m_to_n";
    this->set_name(name);
    this->set_short_name(name);

    if (buffer_size == 0)
    {
        std::stringstream message;
        message << "'buffer_size' has to be greater than 0 ('buffer_size' = " << buffer_size << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (n_elmts.size() == 0)
    {
        std::stringstream message;
        message << "'n_elmts.size()' has to be greater than 0 ('n_elmts.size()' = " << n_elmts.size() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    for (size_t e = 0; e < n_elmts.size(); e++)
    {
        if (n_elmts[e] == 0)
        {
            std::stringstream message;
            message << "'n_elmts[e]' has to be greater than 0 ('e' = " << e << ", 'n_elmts[e]' = " << n_elmts[e]
                    << ").";
            throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
        }
    }

    for (auto& a : *this->first.get())
        a = 0;
    for (auto& a : *this->last.get())
        a = 0;

    this->init();
}

Adaptor_m_to_n::Adaptor_m_to_n(const size_t n_elmts,
                               const std::type_index datatype,
                               const size_t buffer_size,
                               const bool active_waiting)
  : Adaptor_m_to_n(std::vector<size_t>(1, n_elmts),
                   std::vector<std::type_index>(1, datatype),
                   buffer_size,
                   active_waiting)
{
}

void
Adaptor_m_to_n::init()
{
    const std::string name = "Adaptor_m_to_n";
    this->set_name(name);
    this->set_short_name(name);
    this->set_single_wave(true);

    auto& p1 = this->create_task("push");
    std::vector<size_t> p1s_in;
    for (size_t s = 0; s < this->n_sockets; s++)
        p1s_in.push_back(this->create_socket_in(p1, "in" + std::to_string(s), this->n_elmts[s], this->datatype[s]));

    this->create_codelet(p1,
                         [p1s_in](Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& adp = static_cast<Adaptor_m_to_n&>(m);
                             if (adp.is_no_copy_push())
                             {
                                 adp.wait_push();
                                 // for debug mode coherence
                                 for (size_t s = 0; s < t.sockets.size() - 1; s++)
                                     t.sockets[s]->dataptr = adp.get_empty_buffer(s);
                             }
                             else
                             {
                                 std::vector<const int8_t*> sockets_dataptr(p1s_in.size());
                                 for (size_t s = 0; s < p1s_in.size(); s++)
                                     sockets_dataptr[s] = t[p1s_in[s]].get_dataptr<const int8_t>();
                                 adp.push(sockets_dataptr, frame_id);
                             }
                             return runtime::status_t::SUCCESS;
                         });

    auto& p2 = this->create_task("pull");
    std::vector<size_t> p2s_out;
    for (size_t s = 0; s < this->n_sockets; s++)
        p2s_out.push_back(this->create_socket_out(p2, "out" + std::to_string(s), this->n_elmts[s], this->datatype[s]));

    this->create_codelet(p2,
                         [p2s_out](Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& adp = static_cast<Adaptor_m_to_n&>(m);
                             if (adp.is_no_copy_pull())
                             {
                                 adp.wait_pull();
                                 // for debug mode coherence
                                 for (size_t s = 0; s < t.sockets.size() - 1; s++)
                                     t.sockets[s]->_bind(adp.get_filled_buffer(s));
                             }
                             else
                             {
                                 std::vector<int8_t*> sockets_dataptr(p2s_out.size());
                                 for (size_t s = 0; s < p2s_out.size(); s++)
                                     sockets_dataptr[s] = t[p2s_out[s]].get_dataptr<int8_t>();
                                 adp.pull(sockets_dataptr, frame_id);
                             }
                             return runtime::status_t::SUCCESS;
                         });
}

size_t
Adaptor_m_to_n::get_n_elmts(const size_t sid) const
{
    return this->n_elmts[sid];
}

size_t
Adaptor_m_to_n::get_n_bytes(const size_t sid) const
{
    return this->n_bytes[sid];
}

std::type_index
Adaptor_m_to_n::get_datatype(const size_t sid) const
{
    return this->datatype[sid];
}

bool
Adaptor_m_to_n::is_empty(const size_t id)
{
    return (*this->first)[id] == (*this->last)[id];
}

bool
Adaptor_m_to_n::is_full(const size_t id)
{
    // return (size_t)std::abs((int)(*this->last)[id] - (int)(*this->first)[id]) == this->buffer_size;
    return !this->n_free_slots(id);
}

size_t
Adaptor_m_to_n::n_fill_slots(const size_t id)
{
    return (*this->last)[id] - (*this->first)[id];
}

size_t
Adaptor_m_to_n::n_free_slots(const size_t id)
{
    return this->buffer_size - this->n_fill_slots(id);
}

}
}
