/*!
 * \file
 * \brief Class runtime::Task.
 */
#ifndef TASK_HPP_
#define TASK_HPP_

#include <chrono>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Tools/Interface/Interface_clone.hpp"
#include "Tools/Interface/Interface_reset.hpp"
#include "Tools/System/memory.hpp"

namespace spu
{
namespace module
{
class Module;
}

namespace tools
{
class Buffer_allocator;
};

namespace runtime
{
class Socket;
class Pipeline;

enum status_t : int
{
    SUCCESS = 0,
    FAILURE = 1,
    FAILURE_STOP = -1,
    UNKNOWN = -2
};

enum class socket_t : uint8_t
{
    SIN,
    SOUT,
    SFWD
};

enum class datatype_t : uint8_t
{
    F64,
    F32,
    S64,
    S32,
    S16,
    S8,
    U64,
    U32,
    U16,
    U8
};

static std::unordered_map<int, std::string> status_t_to_string = { { 0, "SUCCESS" },
                                                                   { 1, "FAILURE" },
                                                                   { -1, "FAILURE_STOP" },
                                                                   { -2, "UNKNOWN" },
                                                                   { std::numeric_limits<int>::min(), "SKIPPED" } };

class Task
  : public tools::Interface_clone
  , public tools::Interface_reset
{
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Socket;
    friend Pipeline;
    friend module::Module;
    friend tools::Buffer_allocator;
#endif

  protected:
    module::Module* module;
    const std::string name;
    bool stats;
    bool fast;
    bool debug;
    bool outbuffers_allocated;
    bool debug_hex;
    bool replicable;
    int32_t debug_limit;
    uint8_t debug_precision;
    int32_t debug_frame_max;
    std::function<int(module::Module& m, Task& t, const size_t frame_id)> codelet;
    size_t n_input_sockets;
    size_t n_output_sockets;
    size_t n_fwd_sockets;

    std::vector<int> status;

    // stats
    uint32_t n_calls;
    std::chrono::nanoseconds duration_total;
    std::chrono::nanoseconds duration_min;
    std::chrono::nanoseconds duration_max;

    std::vector<std::string> timers_name;
    std::vector<uint32_t> timers_n_calls;
    std::vector<std::chrono::nanoseconds> timers_total;
    std::vector<std::chrono::nanoseconds> timers_min;
    std::vector<std::chrono::nanoseconds> timers_max;

    Socket* last_input_socket;

    // precomputed values to speedup the task execution
    std::vector<int8_t*> sockets_dataptr_init;
    std::vector<size_t> sockets_databytes_per_frame;
    std::vector<std::vector<int8_t>> sockets_data;

  public:
    std::vector<std::shared_ptr<Socket>> fake_input_sockets;
    std::vector<std::shared_ptr<Socket>> sockets;

    Task(module::Module& module,
         const std::string& name,
         const bool stats = false,
         const bool fast = false,
         const bool debug = false,
         const bool outbuffers_allocated = false);

    virtual ~Task() = default;

    void reset();

    void set_stats(const bool stats);
    void set_fast(const bool fast);
    void set_debug(const bool debug);
    void set_debug_hex(const bool debug_hex);
    void set_debug_limit(const uint32_t limit);
    void set_debug_precision(const uint8_t prec);
    void set_debug_frame_max(const uint32_t limit);
    void set_replicability(const bool replicable);
    void set_outbuffers_allocated(const bool outbuffers_allocated);

    inline bool is_stats() const;
    inline bool is_fast() const;
    inline bool is_debug() const;
    inline bool is_debug_hex() const;
    inline bool is_last_input_socket(const Socket& s_in) const;
    inline bool is_outbuffers_allocated() const;
    bool is_stateless() const;
    bool is_stateful() const;
    bool is_replicable() const;
    bool can_exec() const;

    inline module::Module& get_module() const;
    inline std::string get_name() const;
    inline uint32_t get_n_calls() const;

    // get stats
    std::chrono::nanoseconds get_duration_total() const;
    std::chrono::nanoseconds get_duration_avg() const;
    std::chrono::nanoseconds get_duration_min() const;
    std::chrono::nanoseconds get_duration_max() const;
    const std::vector<std::string>& get_timers_name() const;
    const std::vector<uint32_t>& get_timers_n_calls() const;
    const std::vector<std::chrono::nanoseconds>& get_timers_total() const;
    const std::vector<std::chrono::nanoseconds>& get_timers_min() const;
    const std::vector<std::chrono::nanoseconds>& get_timers_max() const;

    size_t get_n_input_sockets() const;
    size_t get_n_output_sockets() const;
    size_t get_n_fwd_sockets() const;
    size_t get_n_static_input_sockets() const;

    const std::vector<int>& exec(const int frame_id = -1, const bool managed_memory = true);

    inline Socket& operator[](const size_t id);
    Socket& operator[](const std::string& sck_name);

    inline void update_timer(const size_t id, const std::chrono::nanoseconds& duration);

    Task* clone() const;

    inline const std::vector<int>& get_status() const;

    void bind(Socket& s_out, const int priority = -1); // deprecated
    void operator=(Socket& s_out);
    size_t unbind(Socket& s_out);

    void bind(Task& t_out, const int priority = -1); // deprecated
    void operator=(Task& t_out);
    size_t unbind(Task& t_out);

  protected:
    void _bind(Socket& s_out, const int priority = -1);
    void _bind(Task& t_out, const int priority = -1);

    void _exec(const int frame_id = -1, const bool managed_memory = true);

    void register_timer(const std::string& key);

    template<typename T>
    size_t create_2d_socket_in(const std::string& name, const size_t n_rows, const size_t n_cols);
    size_t create_2d_socket_in(const std::string& name,
                               const size_t n_rows,
                               const size_t n_cols,
                               const std::type_index& datatype);
    size_t create_2d_socket_in(const std::string& name,
                               const size_t n_rows,
                               const size_t n_cols,
                               const datatype_t datatype);

    template<typename T>
    size_t create_2d_socket_out(const std::string& name,
                                const size_t n_rows,
                                const size_t n_cols,
                                const bool hack_status = false);
    size_t create_2d_socket_out(const std::string& name,
                                const size_t n_rows,
                                const size_t n_cols,
                                const std::type_index& datatype,
                                const bool hack_status = false);
    size_t create_2d_socket_out(const std::string& name,
                                const size_t n_rows,
                                const size_t n_cols,
                                const datatype_t datatype,
                                const bool hack_status = false);

    template<typename T>
    size_t create_2d_socket_fwd(const std::string& name, const size_t n_rows, const size_t n_cols);
    size_t create_2d_socket_fwd(const std::string& name,
                                const size_t n_rows,
                                const size_t n_cols,
                                const std::type_index& datatype);
    size_t create_2d_socket_fwd(const std::string& name,
                                const size_t n_rows,
                                const size_t n_cols,
                                const datatype_t datatype);

    void create_codelet(std::function<int(module::Module& m, Task& t, const size_t frame_id)>& codelet);

    void update_n_frames(const size_t old_n_frames, const size_t new_n_frames);

    void update_n_frames_per_wave(const size_t old_n_frames_per_wave, const size_t new_n_frames_per_wave);

    // Function to allocate task's out sockets
    void allocate_outbuffers();
    // Function to deallocate all tasks out buffers
    void deallocate_outbuffers();

  private:
    template<typename T>
    inline Socket& create_2d_socket(const std::string& name,
                                    const size_t n_rows,
                                    const size_t n_cols,
                                    const socket_t type,
                                    const bool hack_status = false);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Runtime/Task/Task.hxx"
#endif

#endif /* TASK_HPP_ */
