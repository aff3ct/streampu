#include <algorithm>
#include <iterator>
#include <sstream>
#include <type_traits>
#include <utility>

#include "Runtime/Socket/Socket.hpp"
#include "Tools/Exception/exception.hpp"
#ifdef SPU_SHOW_DEPRECATED
#include "Tools/Display/rang_format/rang_format.h"
#ifdef SPU_STACKTRACE
#include <cpptrace/cpptrace.hpp>
#endif
#endif

namespace spu
{
namespace runtime
{
static std::unordered_map<std::type_index, std::string> type_to_string = {
    { typeid(int8_t), "int8" },     { typeid(uint8_t), "uint8" },   { typeid(int16_t), "int16" },
    { typeid(uint16_t), "uint16" }, { typeid(int32_t), "int32" },   { typeid(uint32_t), "uint32" },
    { typeid(int64_t), "int64" },   { typeid(uint64_t), "uint64" }, { typeid(float), "float32" },
    { typeid(double), "float64" }
};

static std::unordered_map<std::type_index, uint8_t> type_to_size = { { typeid(int8_t), 1 },  { typeid(uint8_t), 1 },
                                                                     { typeid(int16_t), 2 }, { typeid(uint16_t), 2 },
                                                                     { typeid(int32_t), 4 }, { typeid(uint32_t), 4 },
                                                                     { typeid(int64_t), 8 }, { typeid(uint64_t), 8 },
                                                                     { typeid(float), 4 },   { typeid(double), 8 } };

Socket::Socket(Task& task,
               const std::string& name,
               const std::type_index datatype,
               const std::pair<size_t, size_t> databytes_per_dim,
               const socket_t type,
               const bool fast,
               void* dataptr)
  : task(task)
  , name(name)
  , datatype(datatype)
  , databytes(std::get<0>(databytes_per_dim) * std::get<1>(databytes_per_dim))
  , fast(fast)
  , dataptr(dataptr)
  , rowsptr(nullptr)
  , n_rows(std::get<0>(databytes_per_dim))
  , start_row(0)
  , bound_socket(nullptr)
  , type(type)
{
    if (databytes % type_to_size[datatype] != 0)
    {
        std::stringstream message;
        message << "'databytes % type_to_size[datatype]' has to be equal to 0 ("
                << "'databytes' = " << databytes << ", "
                << "'type_to_size[datatype]' = " << type_to_size[datatype] << ", "
                << "'datatype' = " << type_to_string[datatype] << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    this->rowsptr = new void*[this->n_rows];
}

Socket::Socket(Task& task,
               const std::string& name,
               const std::type_index datatype,
               const size_t databytes,
               const socket_t type,
               const bool fast,
               void* dataptr)
  : Socket(task, name, datatype, { 1, databytes }, type, fast, dataptr)
{
}

Socket::~Socket()
{
    this->reset();
    this->rowsptr -= this->start_row;
    delete[] rowsptr;
}

const std::string&
Socket::get_name() const
{
    return name;
}

const std::type_index&
Socket::get_datatype() const
{
    return datatype;
}

const std::string&
Socket::get_datatype_string() const
{
    return type_to_string[datatype];
}

uint8_t
Socket::get_datatype_size() const
{
    return type_to_size[datatype];
}

size_t
Socket::get_databytes() const
{
    return databytes;
}

size_t
Socket::get_n_elmts() const
{
    return get_databytes() / (size_t)get_datatype_size();
}

size_t
Socket::get_n_rows() const
{
    return this->n_rows;
}

void*
Socket::_get_dataptr(const size_t start_col) const
{
    uint8_t* ptr = (uint8_t*)dataptr;
    return (void*)(ptr + start_col);
}

void*
Socket::get_dataptr(const size_t start_col) const
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning
              << "Deprecated: 'Socket::get_dataptr' should be replaced by 'Socket::get_dataptr<T>'." << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    return this->_get_dataptr(start_col);
}

void*
Socket::_get_dptr(const size_t start_col) const
{
    return this->get_dataptr(start_col);
}

void*
Socket::get_dptr(const size_t start_col) const
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Socket::get_dptr' should be replaced by 'Socket::get_dptr<T>'."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    return this->_get_dptr(start_col);
}

void**
Socket::_get_2d_dataptr(const size_t start_row, const size_t start_col)
{
    assert(start_row < this->get_n_rows());
    const size_t n_cols = this->get_databytes() / this->get_n_rows();
    assert(start_col < n_cols);

    this->rowsptr -= this->start_row;
    uint8_t* dptr = (uint8_t*)get_dataptr() + start_col;
    for (size_t r = 0; r < this->get_n_rows(); r++)
    {
        this->rowsptr[r] = (void*)dptr;
        dptr += n_cols;
    }

    this->start_row = start_row;
    this->rowsptr += this->start_row;

    return this->rowsptr;
}

void**
Socket::get_2d_dataptr(const size_t start_row, const size_t start_col)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning
              << "Deprecated: 'Socket::get_2d_dataptr' should be replaced by 'Socket::get_2d_dataptr<T>'." << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    return this->_get_2d_dataptr(start_row, start_col);
}

void**
Socket::_get_2d_dptr(const size_t start_row, const size_t start_col)
{
    return this->get_2d_dataptr(start_row, start_col);
}

void**
Socket::get_2d_dptr(const size_t start_row, const size_t start_col)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning
              << "Deprecated: 'Socket::get_2d_dptr' should be replaced by 'Socket::get_2d_dptr<T>'." << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    return this->_get_2d_dptr(start_row, start_col);
}

template<typename T>
T*
Socket::get_dataptr(const size_t start_col) const
{
#ifndef SPU_FAST
    if (this->get_type() == socket_t::SIN && !std::is_const<T>::value)
    {
        std::stringstream message;
        message << "'This is an input socket and the 'T' template should be 'const' ("
                << "'T' = " << typeid(T).name() << ", "
                << "'datatype' = " << type_to_string[this->datatype] << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif

    return static_cast<T*>(this->_get_dataptr(start_col * sizeof(T)));
}

template<typename T>
T*
Socket::get_dptr(const size_t start_col) const
{
    return this->template get_dataptr<T>(start_col);
}

template<typename T>
T**
Socket::get_2d_dataptr(const size_t start_row, const size_t start_col)
{
#ifndef SPU_FAST
    if (this->get_type() == socket_t::SIN && !std::is_const<T>::value)
    {
        std::stringstream message;
        message << "'This is an input socket and the 'T' template should be 'const' ("
                << "'T' = " << typeid(T).name() << ", "
                << "'datatype' = " << type_to_string[this->datatype] << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif

    return (T**)(this->_get_2d_dataptr(start_row, start_col * sizeof(T)));
}

template<typename T>
T**
Socket::get_2d_dptr(const size_t start_row, const size_t start_col)
{
    return this->template get_2d_dataptr<T>(start_row, start_col);
}

Socket::buffer&
Socket::get_out_buffer()
{
    return this->out_buffer;
}

bool
Socket::is_fast() const
{
#ifndef SPU_FAST
    return fast;
#else
    return true;
#endif
}

Task&
Socket::get_task() const
{
    return this->task;
}

const std::vector<Socket*>&
Socket::get_bound_sockets() const
{
    return this->bound_sockets;
}

Socket&
Socket::get_bound_socket()
{
    if (this->bound_socket == nullptr)
    {
        std::stringstream message;
        message << "bound_socket can't be nullptr.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    return *this->bound_socket;
}

const Socket&
Socket::get_bound_socket() const
{
    if (this->bound_socket == nullptr)
    {
        std::stringstream message;
        message << "bound_socket can't be nullptr.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    return *this->bound_socket;
}

socket_t
Socket::get_type() const
{
    return this->type;
}

void
Socket::set_fast(const bool fast)
{
    this->fast = fast;
}

void
Socket::_bind(Socket& s_out, const int priority)
{
#ifndef SPU_FAST
    if (!is_fast())
    {
        if (s_out.datatype != this->datatype)
        {
            std::stringstream message;
            message << "'s_out.datatype' has to be equal to 'datatype' ("
                    << "'s_out.datatype' = " << type_to_string[s_out.datatype] << ", "
                    << "'s_out.name' = " << s_out.get_name() << ", "
                    << "'s_out.task.name' = " << s_out.task.get_name() << ", "
                    << "'datatype' = " << type_to_string[this->datatype] << ", "
                    << "'name' = " << get_name() << ", "
                    << "'task.name' = " << task.get_name() << ").";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

        if (s_out.databytes != this->databytes)
        {
            std::stringstream message;
            message << "'s_out.databytes' has to be equal to 'databytes' ("
                    << "'s_out.databytes' = " << s_out.databytes << ", "
                    << "'s_out.name' = " << s_out.get_name() << ", "
                    << "'s_out.task.name' = " << s_out.task.get_name() << ", "
                    << "'databytes' = " << this->databytes << ", "
                    << "'name' = " << get_name() << ", "
                    << "'task.name' = " << task.get_name() << ").";

            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

        if (s_out.get_task().is_outbuffers_allocated() && s_out.dataptr == nullptr)
        {
            std::stringstream message;
            message << "'s_out.dataptr' can't be NULL.";
            message << "'s_out.dataptr' can't be NULL." << "'s_out.name' = " << s_out.get_name() << ", "
                    << "'s_out.task.name' = " << s_out.task.get_name() << ","
                    << "Flag value : " << s_out.get_task().is_outbuffers_allocated() << "," << "'name' = " << get_name()
                    << ", " << "'task.name' = " << task.get_name() << ").";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
    }
#endif

    if (this->bound_socket == &s_out) this->unbind(s_out);

#ifndef SPU_FAST
    if (this->bound_socket != nullptr && this->get_type() == socket_t::SIN)
    {
        std::stringstream message;
        message << "This socket is already connected ("
                << "'bound_socket->databytes' = " << this->bound_socket->databytes << ", "
                << "'bound_socket->name' = " << this->bound_socket->get_name() << ", "
                << "'bound_socket->task.name' = " << this->bound_socket->task.get_name() << ", "
                << "'s_out.databytes' = " << s_out.databytes << ", "
                << "'s_out.name' = " << s_out.get_name() << ", "
                << "'s_out.task.name' = " << s_out.task.get_name() << ", "
                << "'databytes' = " << this->databytes << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif

    this->bound_socket = &s_out;

#ifndef SPU_FAST
    if (std::find(s_out.bound_sockets.begin(), s_out.bound_sockets.end(), this) != s_out.bound_sockets.end())
    {
        std::stringstream message;
        message << "It is not possible to bind the same socket twice ("
                << "'s_out.databytes' = " << s_out.databytes << ", "
                << "'s_out.name' = " << s_out.get_name() << ", "
                << "'s_out.task.name' = " << s_out.task.get_name() << ", "
                << "'databytes' = " << this->databytes << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif

    if ((size_t)priority > s_out.bound_sockets.size() || priority == -1)
        s_out.bound_sockets.push_back(this);
    else
        s_out.bound_sockets.insert(s_out.bound_sockets.begin() + priority, this);
    this->dataptr = s_out.dataptr;
}

void
Socket::bind(Socket& s_out, const int priority)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Socket::bind()' should be replaced by 'Socket::operator='."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    this->_bind(s_out, priority);
}

void
Socket::operator()(Socket& s_out, const int priority)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Socket::operator()' should be replaced by 'Socket::operator='."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    this->_bind(s_out, priority);
}

template<typename T>
void
Socket::operator=(const void* array)
{
#ifndef SPU_FAST
    if (this->get_type() == socket_t::SIN || this->get_type() == socket_t::SFWD)
#endif
        this->_bind(array);
#ifndef SPU_FAST
    else
    {
        std::stringstream message;
        message << "Current socket have to be an input or forward socket ("
                << "'datatype' = " << type_to_string[this->datatype] << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ", "
                << "'type' = "
                << (get_type() == socket_t::SIN    ? "SIN"
                    : get_type() == socket_t::SFWD ? "SFWD"
                                                   : "SOUT")
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
}

template<typename T>
void
Socket::operator=(void* array)
{
#ifndef SPU_FAST
    if (this->get_type() == socket_t::SIN || this->get_type() == socket_t::SFWD)
#endif
        this->_bind(array);
#ifndef SPU_FAST
    else
    {
        std::stringstream message;
        message << "Current socket have to be an input or forward socket ("
                << "'datatype' = " << type_to_string[this->datatype] << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ", "
                << "'type' = "
                << (get_type() == socket_t::SIN    ? "SIN"
                    : get_type() == socket_t::SFWD ? "SFWD"
                                                   : "SOUT")
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
}

template<typename T>
void
Socket::operator=(const T* array)
{
#ifndef SPU_FAST
    if (this->get_type() == socket_t::SIN || this->get_type() == socket_t::SFWD)
#endif
        this->_bind(array);
#ifndef SPU_FAST
    else
    {
        std::stringstream message;
        message << "Current socket have to be an input or forward socket ("
                << "'datatype' = " << type_to_string[this->datatype] << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ", "
                << "'type' = "
                << (get_type() == socket_t::SIN    ? "SIN"
                    : get_type() == socket_t::SFWD ? "SFWD"
                                                   : "SOUT")
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
}

template<typename T>
void
Socket::operator=(T* array)
{
#ifndef SPU_FAST
    if (this->get_type() == socket_t::SIN || this->get_type() == socket_t::SFWD)
#endif
        this->_bind(array);
#ifndef SPU_FAST
    else
    {
        std::stringstream message;
        message << "Current socket have to be an input or forward socket ("
                << "'datatype' = " << type_to_string[this->datatype] << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ", "
                << "'type' = "
                << (get_type() == socket_t::SIN    ? "SIN"
                    : get_type() == socket_t::SFWD ? "SFWD"
                                                   : "SOUT")
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
}

template<typename T, class A>
void
Socket::operator=(const std::vector<T, A>& vector)
{
#ifndef SPU_FAST
    if (this->get_type() == socket_t::SIN || this->get_type() == socket_t::SFWD)
#endif
        this->_bind(vector);
#ifndef SPU_FAST
    else
    {
        std::stringstream message;
        message << "Current socket have to be an input or forward socket ("
                << "'datatype' = " << type_to_string[this->datatype] << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ", "
                << "'type' = "
                << (get_type() == socket_t::SIN    ? "SIN"
                    : get_type() == socket_t::SFWD ? "SFWD"
                                                   : "SOUT")
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
}

template<typename T, class A>
void
Socket::operator=(std::vector<T, A>& vector)
{
#ifndef SPU_FAST
    if (this->get_type() == socket_t::SIN || this->get_type() == socket_t::SFWD)
#endif
        this->_bind(vector);
#ifndef SPU_FAST
    else
    {
        std::stringstream message;
        message << "Current socket have to be an input or forward socket ("
                << "'datatype' = " << type_to_string[this->datatype] << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ", "
                << "'type' = "
                << (get_type() == socket_t::SIN    ? "SIN"
                    : get_type() == socket_t::SFWD ? "SFWD"
                                                   : "SOUT")
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
}

void
Socket::operator=(Socket& s)
{
    if ((s.get_type() == socket_t::SOUT || s.get_type() == socket_t::SFWD) &&
        (this->get_type() == socket_t::SIN || this->get_type() == socket_t::SFWD))
        this->_bind(s);
    else if ((s.get_type() == socket_t::SIN || s.get_type() == socket_t::SFWD) &&
             (this->get_type() == socket_t::SOUT || this->get_type() == socket_t::SFWD))
        s._bind(*this);
    // Socket forward bind
    else if (s.get_type() == socket_t::SFWD && this->get_type() == socket_t::SFWD)
        this->_bind(s);
#ifndef SPU_FAST
    else
    {
        std::stringstream message;
        message << "Binding of [output and input] or [forward and input] or [forward and forward] socket is required ("
                << "'s.datatype' = " << type_to_string[s.datatype] << ", "
                << "'s.name' = " << s.get_name() << ", "
                << "'s.task.name' = " << s.task.get_name() << ", "
                << "'s.type' = "
                << (s.get_type() == socket_t::SIN    ? "SIN"
                    : s.get_type() == socket_t::SFWD ? "SFWD"
                                                     : "SOUT")
                << ", "
                << "'datatype' = " << type_to_string[this->datatype] << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ", "
                << "'type' = "
                << (get_type() == socket_t::SIN    ? "SIN"
                    : get_type() == socket_t::SFWD ? "SFWD"
                                                   : "SOUT")
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
}

void
Socket::operator=(Task& t)
{
#ifndef SPU_FAST
    if (this->get_type() == socket_t::SOUT || this->get_type() == socket_t::SFWD)
#endif
        t._bind(*this);
#ifndef SPU_FAST
    else
    {
        std::stringstream message;
        message << "The current socket should be and output socket ("
                << "'datatype' = " << type_to_string[this->datatype] << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ", "
                << "'type' = "
                << (get_type() == socket_t::SIN    ? "SIN"
                    : get_type() == socket_t::SFWD ? "SFWD"
                                                   : "SOUT")
                << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
}

template<typename T, class A>
void
Socket::_bind(const std::vector<T, A>& vector)
{
    this->_bind(const_cast<std::vector<T, A>&>(vector));
}

template<typename T, class A>
void
Socket::bind(const std::vector<T, A>& vector)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Socket::bind()' should be replaced by 'Socket::operator='."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    this->_bind(vector);
}

template<typename T, class A>
void
Socket::_bind(std::vector<T, A>& vector)
{
#ifndef SPU_FAST
    if (is_fast())
#endif
        this->dataptr = static_cast<void*>(vector.data());
#ifndef SPU_FAST
    else
    {
        if (vector.size() != this->get_n_elmts())
        {
            std::stringstream message;
            message << "'vector.size()' has to be equal to 'get_n_elmts()' ('vector.size()' = " << vector.size()
                    << ", 'get_n_elmts()' = " << get_n_elmts() << ", 'name' = " << get_name() << ", 'task.name' = "
                    << task.get_name()
                    //			        << ", 'task.module.name' = " << task.get_module_name()
                    << ").";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

        this->_bind(vector.data());
    }
#endif
}

template<typename T, class A>
void
Socket::bind(std::vector<T, A>& vector)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Socket::bind()' should be replaced by 'Socket::operator='."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    this->_bind(vector);
}

template<typename T, class A>
void
Socket::operator()(std::vector<T, A>& vector)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Socket::operator()' should be replaced by 'Socket::operator='."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    this->_bind(vector);
}

template<typename T>
void
Socket::_bind(const T* array)
{
    this->_bind(const_cast<T*>(array));
}

template<typename T>
void
Socket::bind(const T* array)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Socket::bind()' should be replaced by 'Socket::operator='."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    this->_bind(array);
}

template<typename T>
void
Socket::_bind(T* array)
{
#ifndef SPU_FAST
    if (is_fast())
#endif
        this->dataptr = static_cast<void*>(array);
#ifndef SPU_FAST
    else
    {
        if (type_to_string[typeid(T)] != type_to_string[this->datatype])
        {
            std::stringstream message;
            message << "'T' has to be equal to 'datatype' ('T' = " << type_to_string[typeid(T)]
                    << ", 'datatype' = " << type_to_string[this->datatype] << ", 'socket.name' = " << get_name()
                    << ", 'task.name' = " << task.get_name() << ").";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

        this->_bind(static_cast<void*>(array));
    }
#endif
}

template<typename T>
void
Socket::bind(T* array)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Socket::bind()' should be replaced by 'Socket::operator='."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    this->_bind(array);
}

template<typename T>
void
Socket::operator()(T* array)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Socket::operator()' should be replaced by 'Socket::operator='."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    this->_bind(array);
}

void
Socket::_bind(void* dataptr)
{
#ifndef SPU_FAST
    if (!is_fast())
    {
        if (dataptr == nullptr)
        {
            std::stringstream message;
            message << "'s.dataptr' can't be NULL.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        this->check_bound_socket();
    }
#endif
    this->dataptr = dataptr;
}

void
Socket::bind(void* dataptr)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Socket::bind()' should be replaced by 'Socket::operator='."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    this->_bind(dataptr);
}

void
Socket::operator()(void* dataptr)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Socket::operator()' should be replaced by 'Socket::operator='."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    this->_bind(dataptr);
}

void
Socket::reset()
{
    if (this->bound_socket != nullptr) this->unbind(*this->bound_socket);
    // the backforward loop is required here because the 'unbind' method can remove elements in 'bound_sockets' array
    for (int sid = (int)this->bound_sockets.size() - 1; sid >= 0; sid--)
        this->bound_sockets[sid]->unbind(*this);
}

size_t
Socket::unbind(Socket& s_out)
{
#ifndef SPU_FAST
    if (this->bound_socket == nullptr)
    {
        std::stringstream message;
        message << "The current input socket can't be unbound because it is not bound to any output socket ("
                << "'s_out.databytes' = " << s_out.databytes << ", "
                << "'s_out.name' = " << s_out.get_name() << ", "
                << "'s_out.task.name' = " << s_out.task.get_name() << ", "
                << "'databytes' = " << this->databytes << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (this->bound_socket != &s_out)
    {
        std::stringstream message;
        message << "This socket is connected to a different socket than 's_out' ("
                << "'bound_socket->databytes' = " << this->bound_socket->databytes << ", "
                << "'bound_socket->name' = " << this->bound_socket->get_name() << ", "
                << "'bound_socket->task.name' = " << this->bound_socket->task.get_name() << ", "
                << "'s_out.databytes' = " << s_out.databytes << ", "
                << "'s_out.name' = " << s_out.get_name() << ", "
                << "'s_out.task.name' = " << s_out.task.get_name() << ", "
                << "'databytes' = " << this->databytes << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif

    this->bound_socket = nullptr;

    size_t unbind_pos = 0;
    auto it = std::find(s_out.bound_sockets.begin(), s_out.bound_sockets.end(), this);

#ifndef SPU_FAST
    if (it != s_out.bound_sockets.end())
    {
#endif
        unbind_pos = (size_t)std::distance(s_out.bound_sockets.begin(), it);
        s_out.bound_sockets.erase(it);
#ifndef SPU_FAST
    }
    else
    {
        std::stringstream message;
        message << "The 's_out' output socket is not bound to the current input socket ("
                << "'s_out.databytes' = " << s_out.databytes << ", "
                << "'s_out.name' = " << s_out.get_name() << ", "
                << "'s_out.task.name' = " << s_out.task.get_name() << ", "
                << "'databytes' = " << this->databytes << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif

    return unbind_pos;
}

void
Socket::_allocate_buffer()
{
    out_buffer.resize(this->databytes);
    this->dataptr = out_buffer.data();
}

void
Socket::allocate_buffer()
{
    if (this->dataptr != nullptr)
    {
        std::stringstream message;
        message << "The current socket is already allocated (" << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (this->get_type() == socket_t::SIN)
    {
        std::stringstream message;
        message << "The current socket is an input socket and can't be allocated (" << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (this->get_type() == socket_t::SFWD)
    {
        std::stringstream message;
        message << "The current socket is a forward socket and can't be allocated (" << "'name' = " << get_name()
                << ", " << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    _allocate_buffer();
}

void
Socket::free_buffer()
{
    if (this->dataptr == nullptr)
    {
        std::stringstream message;
        message << "The current socket is already deallocated (" << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (this->get_type() == socket_t::SIN)
    {
        std::stringstream message;
        message << "The current socket is an input socket and can't be deallocated (" << "'name' = " << get_name()
                << ", " << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (this->get_type() == socket_t::SFWD)
    {
        std::stringstream message;
        message << "The current socket is a forward socket and can't be deallocated (" << "'name' = " << get_name()
                << ", " << "'task.name' = " << task.get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    out_buffer.clear();
    this->dataptr = nullptr;
}

void
Socket::set_name(const std::string& name)
{
    if (name != this->get_name())
    {
        this->check_bound_socket();
        this->name = name;
    }
}

void
Socket::set_datatype(const std::type_index datatype)
{
    if (datatype != this->get_datatype())
    {
        this->check_bound_socket();
        this->datatype = datatype;
    }
}

void
Socket::set_databytes(const size_t databytes)
{
    if (databytes != this->get_databytes())
    {
        this->check_bound_socket();
        this->databytes = databytes;
    }
}

void
Socket::set_dataptr(void* dataptr)
{
    if (dataptr != this->_get_dataptr())
    {
        this->dataptr = dataptr;
    }
}

void
Socket::set_n_rows(const size_t n_rows)
{
    if (n_rows != this->get_n_rows())
    {
        this->n_rows = n_rows;
        this->rowsptr -= this->start_row;
        delete[] this->rowsptr;

        this->rowsptr = new void*[this->n_rows];
        this->start_row = 0;
    }
}

void
Socket::check_bound_socket()
{
    if (bound_sockets.size() != 0 || bound_socket != nullptr)
    {
        std::stringstream bound_sockets_str;
        if (bound_sockets.size() != 0)
        {
            bound_sockets_str << ", 'bound_sockets' = [";

            for (size_t bs = 0; bs < bound_sockets.size(); bs++)
            {
                bound_sockets_str << "{"
                                  << "'name' = " << bound_sockets[bs]->get_name() << ", "
                                  << "'databytes' = " << bound_sockets[bs]->get_databytes() << ", "
                                  << "'task.name' = " << bound_sockets[bs]->get_task().get_name() << "}";
                if (bs < bound_sockets.size() - 1) bound_sockets_str << ", ";
            }

            bound_sockets_str << "]";
        }
        else if (bound_socket != nullptr)
        {
            bound_sockets_str << ", 'bound_socket' = ";
            bound_sockets_str << "{"
                              << "'name' = " << bound_socket->get_name() << ", "
                              << "'databytes' = " << bound_socket->get_databytes() << ", "
                              << "'task.name' = " << bound_socket->get_task().get_name() << "}";
        }

        std::stringstream message;
        message << "The current socket is already bound ("
                << "'dataptr' = " << _get_dataptr() << ", "
                << "'databytes' = " << get_databytes() << ", "
                << "'datatype' = " << get_datatype_string() << ", "
                << "'name' = " << get_name() << ", "
                << "'task.name' = " << task.get_name() << bound_sockets_str.str() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
}

void
Socket::set_out_buffer(size_t new_data_bytes)
{
    out_buffer.resize(new_data_bytes);
    this->dataptr = ((void*)out_buffer.data());
}

}
}
