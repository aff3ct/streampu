/*!
 * \file
 * \brief Class module::Adaptor_m_to_n.
 */
#ifndef ADAPTOR_M_TO_N_HPP_
#define ADAPTOR_M_TO_N_HPP_

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <typeindex>
#include <vector>

#include "Module/Stateful/Stateful.hpp"
#include "Runtime/Sequence/Sequence.hpp"
#include "Runtime/Socket/Socket.hpp"
#include "Runtime/Task/Task.hpp"
#include "Tools/Interface/Interface_waiting.hpp"

namespace spu
{
namespace module
{

class Adaptor_m_to_n
  : public Stateful
  , public tools::Interface_waiting
{
    friend runtime::Sequence; // Sequence is friend to enable the no copy mode (0 copy)

  protected:
    std::vector<size_t> n_elmts;
    std::vector<size_t> n_bytes;
    const std::vector<std::type_index> datatype;
    const size_t buffer_size;
    const size_t n_sockets;

    std::shared_ptr<std::vector<std::vector<std::vector<int8_t*>>>> buffer;
    std::vector<int8_t*> buffer_to_free;

    std::shared_ptr<std::vector<std::atomic<uint64_t>>> first;
    std::shared_ptr<std::vector<std::atomic<uint64_t>>> last;

    std::shared_ptr<std::atomic<bool>> waiting_canceled;

    bool no_copy_pull;
    bool no_copy_push;

    const bool active_waiting;

    std::shared_ptr<std::vector<std::condition_variable>> cnd_push;
    std::shared_ptr<std::vector<std::mutex>> mtx_push;
    std::shared_ptr<std::vector<std::condition_variable>> cnd_pull;
    std::shared_ptr<std::vector<std::mutex>> mtx_pull;

    int tid_push;
    int tid_pull;
    std::shared_ptr<size_t> n_pushers;
    std::shared_ptr<size_t> n_pullers;
    std::shared_ptr<size_t> n_clones;

    std::shared_ptr<bool> buffers_allocated;
    bool cloned;
    size_t cur_push_id;
    size_t cur_pull_id;

  public:
    inline Adaptor_m_to_n(const size_t n_elmts,
                          const std::type_index datatype,
                          const size_t buffer_size = 1,
                          const bool active_waiting = true);
    inline Adaptor_m_to_n(const std::vector<size_t>& n_elmts,
                          const std::vector<std::type_index>& datatype,
                          const size_t buffer_size = 1,
                          const bool active_waiting = true);
    inline size_t get_n_elmts(const size_t sid = 0) const;
    inline size_t get_n_bytes(const size_t sid = 0) const;
    inline std::type_index get_datatype(const size_t sid = 0) const;
    void reset();
    virtual ~Adaptor_m_to_n();
    virtual Adaptor_m_to_n* clone() const;
    virtual void set_n_frames(const size_t n_frames);

    virtual void wake_up();
    void cancel_waiting();

    void add_pusher();
    void add_puller();
    void alloc_buffers();

  protected:
    void send_cancel_signal();
    inline void init();
    void deep_copy(const Adaptor_m_to_n& m);
    inline bool is_full(const size_t id);
    inline bool is_empty(const size_t id);
    inline size_t n_free_slots(const size_t id);
    inline size_t n_fill_slots(const size_t id);

    void set_no_copy_push(const bool no_copy_push);
    void set_no_copy_pull(const bool no_copy_pull);
    bool is_no_copy_push();
    bool is_no_copy_pull();
    void reset_buffer();

    virtual void push(const std::vector<const int8_t*>& in, const size_t frame_id);
    virtual void pull(const std::vector<int8_t*>& out, const size_t frame_id);

    void wait_push();
    void wait_pull();
    virtual void* get_empty_buffer(const size_t sid);
    virtual void* get_filled_buffer(const size_t sid);
    virtual void* get_empty_buffer(const size_t sid, void* swap_buffer);
    virtual void* get_filled_buffer(const size_t sid, void* swap_buffer);
    virtual void wake_up_pusher();
    virtual void wake_up_puller();
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Stateful/Adaptor/Adaptor_m_to_n.hxx"
#endif

#endif /* ADAPTOR_M_TO_N_HPP_ */
