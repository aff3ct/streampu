#include "Module/Stateful/Adaptor/Adaptor_m_to_n.hpp"
#include "Tools/Math/utils.h"

using namespace spu;
using namespace spu::module;

Adaptor_m_to_n::~Adaptor_m_to_n()
{
    for (auto b : this->buffer_to_free)
        delete[] b;
    if (this->cloned) (*this->n_clones)--;
}

Adaptor_m_to_n*
Adaptor_m_to_n::clone() const
{
    auto m = new Adaptor_m_to_n(*this);
    m->deep_copy(*this);
    return m;
}

void
Adaptor_m_to_n::deep_copy(const Adaptor_m_to_n& m)
{
    Stateful::deep_copy(m);

    if (*this->buffers_allocated)
    {
        std::stringstream message;
        message << "Shared buffers have already been allocated, cloning is no more possible." << std::endl;
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->cloned = true;
    (*this->n_clones)++;
    this->tid_push = -1;
    this->tid_pull = -1;
    this->cur_push_id = -1;
    this->cur_pull_id = -1;

    this->waiting_canceled.reset(new std::atomic<bool>(m.waiting_canceled->load()));
}

void
Adaptor_m_to_n::alloc_buffers()
{
    if (*this->buffers_allocated)
    {
        std::stringstream message;
        message << "Synchronization buffers have already been allocated.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (this->cloned)
    {
        std::stringstream message;
        message << "'alloc_buffers()' cannot be called on a cloned module ('tid_push' = " << this->tid_push
                << ", 'tid_pull' = " << this->tid_pull << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    size_t ppcm = tools::find_smallest_common_multiple(*this->n_pushers, *this->n_pullers);

    if (ppcm > 1000)
    {
        std::stringstream message;
        message << "'ppcm' cannot exceed 1000 ('ppcm' = " << ppcm << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    this->buffer->resize(ppcm,
                         std::vector<std::vector<int8_t*>>(this->n_sockets, std::vector<int8_t*>(this->buffer_size)));

    if (this->buffer_to_free.size())
    {
        std::stringstream message;
        message << "This should never happen.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    // this->buffer_to_free.clear();
    for (size_t d = 0; d < ppcm; d++)
    {
        for (size_t s = 0; s < this->n_sockets; s++)
            for (size_t b = 0; b < this->buffer_size; b++)
            {
                (*this->buffer)[d][s][b] = new int8_t[this->n_frames * this->n_bytes[s]];
                this->buffer_to_free.push_back((*this->buffer)[d][s][b]);
            }
        (*this->first)[d] = 0;
        (*this->last)[d] = 0;
        (*this->counter)[d] = this->buffer_size;
    }

    *this->buffers_allocated = true;
}

void
Adaptor_m_to_n::add_pusher()
{
    if (*this->n_pushers - 1 + *this->n_pullers - 1 >= *this->n_clones)
    {
        std::stringstream message;
        message << "Pusher cannot be added because a clone is missing ('n_pushers' = " << *this->n_pushers
                << ", 'n_pullers' = " << *this->n_pullers << ", 'n_clones' = " << *this->n_clones << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->tid_push = (*this->n_pushers);
    (*this->n_pushers)++;
    this->cur_push_id = this->tid_push;
}

void
Adaptor_m_to_n::add_puller()
{
    if (*this->n_pushers - 1 + *this->n_pullers - 1 >= *this->n_clones)
    {
        std::stringstream message;
        message << "Puller cannot be added because a clone is missing ('n_pushers' = " << *this->n_pushers
                << ", 'n_pullers' = " << *this->n_pullers << ", 'n_clones' = " << *this->n_clones << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->tid_pull = (*this->n_pullers);
    (*this->n_pullers)++;
    this->cur_pull_id = this->tid_pull;
}

void
Adaptor_m_to_n::send_cancel_signal()
{
    *this->waiting_canceled = true;
}

void
Adaptor_m_to_n::reset()
{
    if (!this->cloned)
    {
        *this->waiting_canceled = false;
        for (size_t d = 0; d < this->buffer->size(); d++)
        {
            (*this->first)[d] = 0;
            (*this->last)[d] = 0;
            (*this->counter)[d] = this->buffer_size;
        }
    }
    this->cur_push_id = (size_t)this->tid_push;
    this->cur_pull_id = (size_t)this->tid_pull;
    this->reset_buffer();
}

void
Adaptor_m_to_n::set_no_copy_pull(const bool no_copy_pull)
{
    this->no_copy_pull = no_copy_pull;
}

void
Adaptor_m_to_n::set_no_copy_push(const bool no_copy_push)
{
    this->no_copy_push = no_copy_push;
}

bool
Adaptor_m_to_n::is_no_copy_pull()
{
    return this->no_copy_pull;
}

bool
Adaptor_m_to_n::is_no_copy_push()
{
    return this->no_copy_push;
}

void
Adaptor_m_to_n::reset_buffer()
{
    if (!this->cloned && *this->buffers_allocated)
    {
        size_t id_buff = 0;
        for (size_t d = 0; d < (*this->buffer).size(); d++)
            for (size_t s = 0; s < this->n_sockets; s++)
                for (size_t b = 0; b < this->buffer_size; b++)
                    (*this->buffer)[d][s][b] = this->buffer_to_free[id_buff++];
    }
}

void
Adaptor_m_to_n::set_n_frames(const size_t n_frames)
{
    const auto old_n_frames = this->get_n_frames();
    if (old_n_frames != n_frames)
    {
        Module::set_n_frames(n_frames);
        if (!this->cloned)
        {
            if (*this->buffers_allocated)
            {
                for (size_t d = 0; d < (*this->buffer).size(); d++)
                {
                    for (size_t s = 0; s < (*this->buffer)[d].size(); s++)
                    {
                        for (size_t b = 0; b < (*this->buffer)[d][s].size(); b++)
                        {
                            auto old_ptr = (*this->buffer)[d][s][b];
                            (*this->buffer)[d][s][b] = new int8_t[this->n_bytes[s] * n_frames];

                            bool found = false;
                            for (size_t bf = 0; bf < this->buffer_to_free.size(); bf++)
                                if (this->buffer_to_free[bf] == old_ptr)
                                {
                                    delete[] this->buffer_to_free[bf];
                                    this->buffer_to_free[bf] = (*this->buffer)[d][s][b];
                                    found = true;
                                    break;
                                }

                            if (found == false)
                            {
                                std::stringstream message;
                                message << "This should never happen.";
                                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
                            }
                        }
                    }
                }
            }
        }
    }
}

// --------------------------------------------------------------------------------------------------------------------

void
Adaptor_m_to_n::push(const std::vector<const int8_t*>& in, const size_t frame_id)
{
    this->wait_push();

    for (size_t s = 0; s < this->n_sockets; s++)
    {
        int8_t* out = (int8_t*)this->get_empty_buffer(s);

        std::copy(
          in[s] + 0 * this->n_bytes[s], in[s] + this->get_n_frames() * this->n_bytes[s], out + 0 * this->n_bytes[s]);
    }

    this->wake_up_puller();
}

void
Adaptor_m_to_n::pull(const std::vector<int8_t*>& out, const size_t frame_id)
{
    this->wait_pull();

    for (size_t s = 0; s < this->n_sockets; s++)
    {
        const int8_t* in = (const int8_t*)this->get_filled_buffer(s);

        std::copy(
          in + 0 * this->n_bytes[s], in + this->get_n_frames() * this->n_bytes[s], out[s] + 0 * this->n_bytes[s]);
    }

    this->wake_up_pusher();
}

void
Adaptor_m_to_n::wait_push()
{
    if (this->tid_push < 0)
    {
        std::stringstream message;
        message << "This instance is not a pusher.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (this->active_waiting)
    {
        while (this->is_full(this->cur_push_id) && !*this->waiting_canceled)
            ;
    }
    else // passive waiting
    {
        if (this->is_full(this->cur_push_id) && !*this->waiting_canceled)
        {
            std::unique_lock<std::mutex> lock((*this->mtx_push.get())[this->cur_push_id]);
            ((*this->cnd_push.get())[this->cur_push_id])
              .wait(lock, [this]() { return !(this->is_full(this->cur_push_id) && !*this->waiting_canceled); });
        }
    }

    if (*this->waiting_canceled) throw tools::waiting_canceled(__FILE__, __LINE__, __func__);
}

void
Adaptor_m_to_n::wait_pull()
{
    if (this->tid_pull < 0)
    {
        std::stringstream message;
        message << "This instance is not a puller.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (this->active_waiting)
    {
        while (this->is_empty(this->cur_pull_id) && !*this->waiting_canceled)
            ;
    }
    else // passive waiting
    {
        if (this->is_empty(this->cur_pull_id) && !*this->waiting_canceled)
        {
            std::unique_lock<std::mutex> lock((*this->mtx_pull.get())[this->cur_pull_id]);
            ((*this->cnd_pull.get())[this->cur_pull_id])
              .wait(lock, [this]() { return !(this->is_empty(this->cur_pull_id) && !*this->waiting_canceled); });
        }
    }

    if (this->is_empty(this->cur_pull_id) && *this->waiting_canceled)
        throw tools::waiting_canceled(__FILE__, __LINE__, __func__);
}

void*
Adaptor_m_to_n::get_empty_buffer(const size_t sid)
{
#ifndef SPU_FAST
    if (!*this->buffers_allocated)
    {
        std::stringstream message;
        message << "You need to call 'alloc_buffers()' before to change 'get_empty_buffer()'.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
    return (void*)(*this->buffer)[this->cur_push_id][sid][(*this->last)[this->cur_push_id]];
}

void*
Adaptor_m_to_n::get_filled_buffer(const size_t sid)
{
#ifndef SPU_FAST
    if (!*this->buffers_allocated)
    {
        std::stringstream message;
        message << "You need to call 'alloc_buffers()' before to change 'get_filled_buffer()'.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
    return (void*)(*this->buffer)[this->cur_pull_id][sid][(*this->first)[this->cur_pull_id]];
}

void*
Adaptor_m_to_n::get_empty_buffer(const size_t sid, void* swap_buffer)
{
#ifndef SPU_FAST
    if (!*this->buffers_allocated)
    {
        std::stringstream message;
        message << "You need to call 'alloc_buffers()' before to change 'get_empty_buffer()'.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
    void* empty_buffer = (void*)(*this->buffer)[this->cur_push_id][sid][(*this->last)[this->cur_push_id]];
    (*this->buffer)[this->cur_push_id][sid][(*this->last)[this->cur_push_id]] = (int8_t*)swap_buffer;
    return empty_buffer;
}

void*
Adaptor_m_to_n::get_filled_buffer(const size_t sid, void* swap_buffer)
{
#ifndef SPU_FAST
    if (!*this->buffers_allocated)
    {
        std::stringstream message;
        message << "You need to call 'alloc_buffers()' before to change 'get_filled_buffer()'.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
    void* filled_buffer = (void*)(*this->buffer)[this->cur_pull_id][sid][(*this->first)[this->cur_pull_id]];
    (*this->buffer)[this->cur_pull_id][sid][(*this->first)[this->cur_pull_id]] = (int8_t*)swap_buffer;
    return filled_buffer;
}

void
Adaptor_m_to_n::wake_up_puller()
{
    (*this->last)[this->cur_push_id] = ((*this->last)[this->cur_push_id] + 1) % this->buffer_size;
    (*this->counter)[this->cur_push_id]--; // atomic fetch sub

    if (!this->active_waiting) // passive waiting
    {
        if (!this->is_empty(this->cur_push_id))
        {
            std::lock_guard<std::mutex> lock((*this->mtx_pull.get())[this->cur_push_id]);
            (*this->cnd_pull.get())[this->cur_push_id].notify_one();
        }
    }

    this->cur_push_id = (this->cur_push_id + *this->n_pushers) % this->buffer->size();
}

void
Adaptor_m_to_n::wake_up_pusher()
{
    (*this->first)[this->cur_pull_id] = ((*this->first)[this->cur_pull_id] + 1) % this->buffer_size;
    (*this->counter)[this->cur_pull_id]++; // atomic fetch add

    if (!this->active_waiting) // passive waiting
    {
        if (!this->is_full(this->cur_pull_id))
        {
            std::lock_guard<std::mutex> lock((*this->mtx_push.get())[this->cur_pull_id]);
            (*this->cnd_push.get())[this->cur_pull_id].notify_one();
        }
    }

    this->cur_pull_id = (this->cur_pull_id + *this->n_pullers) % this->buffer->size();
}

void
Adaptor_m_to_n::wake_up()
{
    if (!this->active_waiting) // passive waiting
    {
        for (size_t i = 0; i < this->buffer->size(); i++)
        {
            std::unique_lock<std::mutex> lock((*this->mtx_push.get())[i]);
            (*this->cnd_push.get())[i].notify_all();
        }
        for (size_t i = 0; i < this->buffer->size(); i++)
        {
            std::unique_lock<std::mutex> lock((*this->mtx_pull.get())[i]);
            (*this->cnd_pull.get())[i].notify_all();
        }
    }
}

void
Adaptor_m_to_n::cancel_waiting()
{
    this->send_cancel_signal();
    this->wake_up();
}
