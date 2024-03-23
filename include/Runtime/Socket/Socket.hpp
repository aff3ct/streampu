/*!
 * \file
 * \brief Class runtime::Socket.
 */
#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include <string>
#include <memory>
#include <vector>
#include <cstddef>
#include <typeindex>

#include "Tools/Interface/Interface_reset.hpp"
#include "Runtime/Task/Task.hpp"

namespace aff3ct
{
namespace module
{
class Adaptor_n_to_1;
class Adaptor_1_to_n;
class Subsequence;
}
namespace runtime
{
class Sequence;

class Socket : public tools::Interface_reset
{
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	friend Task;
	friend module::Adaptor_n_to_1;
	friend module::Adaptor_1_to_n;
	friend module::Subsequence;
	friend Sequence;
#endif
protected:
	Task &task;

	std::string          name;
	std::type_index      datatype;
	size_t               databytes;
	bool                 fast;
	void*                dataptr;
	void**               rowsptr;
	size_t               n_rows;
	size_t               start_row;
	std::vector<Socket*> bound_sockets;
	Socket*              bound_socket;
	socket_t             type;

public:
	inline Socket(Task &task, const std::string &name, const std::type_index datatype, const size_t databytes,
	              const socket_t type, const bool fast = false, void *dataptr = nullptr);
	inline Socket(Task &task, const std::string &name, const std::type_index datatype,
	              const std::pair<size_t, size_t> databytes_per_dim, const socket_t type, const bool fast = false,
	              void *dataptr = nullptr);
	virtual inline ~Socket();

	inline const std::string&          get_name           () const;
	inline const std::type_index&      get_datatype       () const;
	inline const std::string&          get_datatype_string() const;
	inline uint8_t                     get_datatype_size  () const;
	inline size_t                      get_databytes      () const;
	inline size_t                      get_n_elmts        () const;
	inline size_t                      get_n_rows         () const;
	inline void*                       get_dataptr        (const size_t start_col = 0) const;
	inline void*                       get_dptr           (const size_t start_col = 0) const;
	inline void**                      get_2d_dataptr     (const size_t start_row = 0, const size_t start_col = 0);
	inline void**                      get_2d_dptr        (const size_t start_row = 0, const size_t start_col = 0);
	inline bool                        is_fast            () const;
	inline Task&                       get_task           () const;
	inline const std::vector<Socket*>& get_bound_sockets  () const;
	inline const Socket&               get_bound_socket   () const;
	inline       Socket&               get_bound_socket   ();
	inline socket_t                    get_type           () const;

	template <typename T>
	inline T* get_dataptr(const size_t start_col = 0) const;

	template <typename T>
	inline T* get_dptr(const size_t start_col = 0) const;

	template <typename T>
	inline T** get_2d_dataptr(const size_t start_row = 0, const size_t start_col = 0);

	template <typename T>
	inline T** get_2d_dptr(const size_t start_row = 0, const size_t start_col = 0);

	inline void set_fast(const bool fast);

	inline void bind(Socket &s_out, const int priority = -1);

	inline void operator()(Socket &s_out, const int priority = -1);

	template <typename T>
	inline void operator=(const void *array);

	template <typename T>
	inline void operator=(void *array);

	template <typename T>
	inline void operator=(const T *array);

	template <typename T>
	inline void operator=(T *array);

	template <typename T, class A = std::allocator<T>>
	inline void operator=(const std::vector<T,A> &vector);

	template <typename T, class A = std::allocator<T>>
	inline void operator=(std::vector<T,A> &vector);

	inline void operator=(Socket &s);

	inline void operator=(Task &t);

	template <typename T, class A = std::allocator<T>>
	inline void bind(const std::vector<T,A> &vector);

	template <typename T, class A = std::allocator<T>>
	inline void bind(std::vector<T,A> &vector);

	template <typename T, class A = std::allocator<T>>
	inline void operator()(std::vector<T,A> &vector);

	template <typename T>
	inline void bind(const T *array);

	template <typename T>
	inline void bind(T *array);

	template <typename T>
	inline void operator()(T *array);

	inline void bind(void* dataptr);

	inline void operator()(void* dataptr);

	inline void reset();

	inline size_t unbind(Socket& s_out);

protected:
	inline void*  _get_dataptr   (const size_t start_col = 0) const;
	inline void*  _get_dptr      (const size_t start_col = 0) const;
	inline void** _get_2d_dataptr(const size_t start_row = 0, const size_t start_col = 0);
	inline void** _get_2d_dptr   (const size_t start_row = 0, const size_t start_col = 0);

	inline void set_name(const std::string &name);

	inline void set_datatype(const std::type_index datatype);

	inline void set_databytes(const size_t databytes);

	inline void set_dataptr(void* dataptr);

	inline void set_n_rows(const size_t n_rows);

private:
	inline void check_bound_socket();
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Runtime/Socket/Socket.hxx"
#endif

#endif /* SOCKET_HPP_ */
