/*!
 * \file
 * \brief Class runtime::Socket.
 */
#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

#include "Runtime/Task/Task.hpp"
#include "Tools/Interface/Interface_reset.hpp"
#include "Tools/System/memory.hpp"

namespace spu
{
namespace module
{
class Adaptor_m_to_n;
class Set;
}

namespace buffer
{
class Buffer_allocator;
}
namespace runtime
{
class Sequence;
class Pipeline;

class Socket : public tools::Interface_reset
{
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Task;
    friend module::Adaptor_m_to_n;
    friend module::Set;
    friend Sequence;
    friend Pipeline;
    friend buffer::Buffer_allocator;
#endif
  protected:
    Task& task;

    std::string name;
    std::type_index datatype;
    size_t databytes;
    bool fast;
    void* dataptr;
    void** rowsptr;
    size_t n_rows;
    size_t start_row;
    std::vector<Socket*> bound_sockets;
    Socket* bound_socket;
    socket_t type;
    // Data management in socket for CPU case
    typedef std::vector<uint8_t, tools::aligned_allocator<uint8_t>> buffer;
    buffer out_buffer;

  public:
    inline Socket(Task& task,
                  const std::string& name,
                  const std::type_index datatype,
                  const size_t databytes,
                  const socket_t type,
                  const bool fast = false,
                  void* dataptr = nullptr);
    inline Socket(Task& task,
                  const std::string& name,
                  const std::type_index datatype,
                  const std::pair<size_t, size_t> databytes_per_dim,
                  const socket_t type,
                  const bool fast = false,
                  void* dataptr = nullptr);
    virtual inline ~Socket();

    inline const std::string& get_name() const;
    inline const std::type_index& get_datatype() const;
    inline const std::string& get_datatype_string() const;
    inline uint8_t get_datatype_size() const;
    inline size_t get_databytes() const;
    inline size_t get_n_elmts() const;
    inline size_t get_n_rows() const;
    inline void* get_dataptr(const size_t start_col = 0) const;                           // deprecated
    inline void* get_dptr(const size_t start_col = 0) const;                              // deprecated
    inline void** get_2d_dataptr(const size_t start_row = 0, const size_t start_col = 0); // deprecated
    inline void** get_2d_dptr(const size_t start_row = 0, const size_t start_col = 0);    // deprecated
    inline bool is_fast() const;
    inline Task& get_task() const;
    inline const std::vector<Socket*>& get_bound_sockets() const;
    inline const Socket& get_bound_socket() const;
    inline Socket& get_bound_socket();
    inline socket_t get_type() const;
    inline buffer& get_out_buffer();

    template<typename T>
    inline T* get_dataptr(const size_t start_col = 0) const;

    template<typename T>
    inline T* get_dptr(const size_t start_col = 0) const;

    template<typename T>
    inline T** get_2d_dataptr(const size_t start_row = 0, const size_t start_col = 0);

    template<typename T>
    inline T** get_2d_dptr(const size_t start_row = 0, const size_t start_col = 0);

    inline void set_fast(const bool fast);

    inline void bind(Socket& s_out, const int priority = -1); // deprecated

    inline void operator()(Socket& s_out, const int priority = -1); // deprecated

    template<typename T, class A = std::allocator<T>>
    inline void bind(const std::vector<T, A>& vector); // deprecated

    template<typename T, class A = std::allocator<T>>
    inline void bind(std::vector<T, A>& vector); // deprecated

    template<typename T, class A = std::allocator<T>>
    inline void operator()(std::vector<T, A>& vector); // deprecated

    template<typename T>
    inline void bind(const T* array); // deprecated

    template<typename T>
    inline void bind(T* array); // deprecated

    template<typename T>
    inline void operator()(T* array); // deprecated

    inline void bind(void* dataptr); // deprecated

    inline void operator()(void* dataptr); // deprecated

    template<typename T>
    inline void operator=(const void* array);

    template<typename T>
    inline void operator=(void* array);

    template<typename T>
    inline void operator=(const T* array);

    template<typename T>
    inline void operator=(T* array);

    template<typename T, class A = std::allocator<T>>
    inline void operator=(const std::vector<T, A>& vector);

    template<typename T, class A = std::allocator<T>>
    inline void operator=(std::vector<T, A>& vector);

    inline void operator=(Socket& s);

    inline void operator=(Task& t);

    inline void reset();

    inline size_t unbind(Socket& s_out);

    inline void allocate_buffer();

    inline void free_buffer();

  protected:
    inline void* _get_dataptr(const size_t start_col = 0) const;
    inline void* _get_dptr(const size_t start_col = 0) const;
    inline void** _get_2d_dataptr(const size_t start_row = 0, const size_t start_col = 0);
    inline void** _get_2d_dptr(const size_t start_row = 0, const size_t start_col = 0);

    inline void _bind(Socket& s_out, const int priority = -1);

    template<typename T, class A = std::allocator<T>>
    inline void _bind(const std::vector<T, A>& vector);

    template<typename T, class A = std::allocator<T>>
    inline void _bind(std::vector<T, A>& vector);

    template<typename T>
    inline void _bind(const T* array);

    template<typename T>
    inline void _bind(T* array);

    inline void _bind(void* dataptr);

    inline void _allocate_buffer();

    inline void set_name(const std::string& name);

    inline void set_datatype(const std::type_index datatype);

    inline void set_databytes(const size_t databytes);

    inline void set_dataptr(void* dataptr);

    inline void set_n_rows(const size_t n_rows);

    inline void set_out_buffer(size_t new_data_bytes);

  private:
    inline void check_bound_socket();
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Runtime/Socket/Socket.hxx"
#endif

#endif /* SOCKET_HPP_ */
