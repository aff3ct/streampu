#include <sstream>

#include "Module/Stateful/Probe/Probe.hpp"
#include "Tools/Exception/exception.hpp"

namespace spu
{
namespace module
{

runtime::Task&
AProbe::operator[](const prb::tsk t)
{
    return Module::operator[]((int)t);
}

runtime::Socket&
AProbe::operator[](const prb::sck::probe s)
{
    assert((*this)[prb::tsk::probe].get_n_input_sockets() == 1);
    return Module::operator[]((int)prb::tsk::probe)[(int)s];
}

runtime::Socket&
AProbe::operator[](const prb::sck::probe_noin s)
{
    assert((*this)[prb::tsk::probe].get_n_input_sockets() == 0);
    return Module::operator[]((int)prb::tsk::probe)[(int)s];
}

runtime::Socket&
AProbe::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
runtime::Task&
Probe<T>::operator[](const prb::tsk t)
{
    return Module::operator[]((int)t);
}

template<typename T>
runtime::Socket&
Probe<T>::operator[](const prb::sck::probe s)
{
    assert((*this)[prb::tsk::probe].get_n_input_sockets() == 1);
    return Module::operator[]((int)prb::tsk::probe)[(int)s];
}

template<typename T>
runtime::Socket&
Probe<T>::operator[](const prb::sck::probe_noin s)
{
    assert((*this)[prb::tsk::probe].get_n_input_sockets() == 0);
    return Module::operator[]((int)prb::tsk::probe)[(int)s];
}

template<typename T>
runtime::Socket&
Probe<T>::operator[](const std::string& tsk_sck)
{
    return Module::operator[](tsk_sck);
}

template<typename T>
Probe<T>::Probe(const size_t socket_size, const std::string& col_name)
  : AProbe()
  , socket_size(socket_size)
  , col_name(col_name)
{
    const std::string name = "Probe<" + col_name + ">";
    this->set_name(name);
    this->set_short_name(name);

    auto& p1 = this->create_task("probe");
    auto p1s_in = (socket_size > 0) ? (int)this->template create_socket_in<T>(p1, "in", this->socket_size) : (int)-1;
    this->create_codelet(p1,
                         [p1s_in](Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& prb = static_cast<Probe<T>&>(m);
                             const T* in = (p1s_in != -1) ? t[p1s_in].template get_dataptr<const T>() : nullptr;
                             prb._probe(in, frame_id);
                             return runtime::status_t::SUCCESS;
                         });
}

template<typename T>
void
Probe<T>::set_n_frames(const size_t n_frames)
{
    const size_t old_n_frames = this->get_n_frames();
    if (old_n_frames != n_frames)
    {
        Module::set_n_frames(n_frames);
        if (reporter != nullptr) reporter->set_n_frames(n_frames);
    }
}

template<typename T>
template<class AT>
void
Probe<T>::probe(const std::vector<T, AT>& in, const int frame_id, const bool managed_memory)
{
    if ((*this)[prb::tsk::probe].get_n_input_sockets() != 1)
    {
        std::stringstream message;
        message << "The number of input sockets has to be equal to 1 ('get_n_input_sockets()' = "
                << (*this)[prb::tsk::probe].get_n_input_sockets() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    (*this)[prb::sck::probe::in].bind(in);
    (*this)[prb::tsk::probe].exec(frame_id, managed_memory);
}

template<typename T>
void
Probe<T>::probe(const T* in, const int frame_id, const bool managed_memory)
{
    if ((*this)[prb::tsk::probe].get_n_input_sockets() != 1)
    {
        std::stringstream message;
        message << "The number of input sockets has to be equal to 1 ('get_n_input_sockets()' = "
                << (*this)[prb::tsk::probe].get_n_input_sockets() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    (*this)[prb::sck::probe::in].bind(in);
    (*this)[prb::tsk::probe].exec(frame_id, managed_memory);
}

template<typename T>
void
Probe<T>::probe(const int frame_id, const bool managed_memory)
{
    if ((*this)[prb::tsk::probe].get_n_input_sockets() != 0)
    {
        std::stringstream message;
        message << "The number of input sockets has to be equal to 0 ('get_n_input_sockets()' = "
                << (*this)[prb::tsk::probe].get_n_input_sockets() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    (*this)[prb::tsk::probe].exec(frame_id, managed_memory);
}

template<typename T>
void
Probe<T>::_probe(const T* /*in*/, const size_t /*frame_id*/)
{
    throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
}

template<typename T>
void
Probe<T>::reset()
{
}

template<typename T>
void
Probe<T>::set_col_unit(const std::string& unit)
{
    this->check_reporter();
    this->reporter->set_col_unit(unit, *this);
}

template<typename T>
void
Probe<T>::set_col_buff_size(const size_t buffer_size)
{
    this->check_reporter();
    this->reporter->set_col_buff_size(buffer_size, *this);
}

template<typename T>
void
Probe<T>::set_col_fmtflags(const std::ios_base::fmtflags ff)
{
    this->check_reporter();
    this->reporter->set_col_fmtflags(ff, *this);
}

template<typename T>
void
Probe<T>::set_col_prec(const size_t precision)
{
    this->check_reporter();
    this->reporter->set_col_prec(precision, *this);
}

template<typename T>
void
Probe<T>::set_col_size(const size_t col_size)
{
    this->check_reporter();
    this->reporter->set_col_size(col_size, *this);
}

template<typename T>
const std::string&
Probe<T>::get_col_name() const
{
    return this->col_name;
}

template<typename T>
size_t
Probe<T>::get_socket_size() const
{
    return this->socket_size;
}

}
}
