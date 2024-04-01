#include <chrono>
#include <sstream>

#include "Module/Relayer/Relayer.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

template<typename T>
Relayer<T>::Relayer(const size_t n_elmts, const size_t ns)
  : Module()
  , n_elmts(n_elmts)
  , ns(ns)
{
    const std::string name = "Relayer";
    this->set_name(name);
    this->set_short_name(name);

    if (n_elmts == 0)
    {
        std::stringstream message;
        message << "'n_elmts' has to be greater than 0 ('n_elmts' = " << n_elmts << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    auto& p1 = this->create_task("relay");
    auto p1s_in = this->template create_socket_in<T>(p1, "in", this->n_elmts);
    auto p1s_out = this->template create_socket_out<T>(p1, "out", this->n_elmts);
    this->create_codelet(
      p1,
      [p1s_in, p1s_out](Module& m, runtime::Task& t, const size_t frame_id) -> int
      {
          auto& rly = static_cast<Relayer&>(m);
          rly._relay(t[p1s_in].template get_dataptr<const T>(), t[p1s_out].template get_dataptr<T>(), frame_id);
          return runtime::status_t::SUCCESS;
      });

    auto& p2 = this->create_task("relayf");
    auto p2s_fwd = this->template create_socket_fwd<T>(p2, "fwd", this->n_elmts);
    this->create_codelet(
      p2,
      [p2s_fwd](Module& m, runtime::Task& t, const size_t frame_id) -> int
      {
          auto& rly_fwd = static_cast<Relayer&>(m);
          rly_fwd._relay(t[p2s_fwd].template get_dataptr<const T>(), t[p2s_fwd].template get_dataptr<T>(), frame_id);
          return runtime::status_t::SUCCESS;
      });
}

template<typename T>
Relayer<T>*
Relayer<T>::clone() const
{
    auto m = new Relayer(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
size_t
Relayer<T>::get_ns() const
{
    return this->ns;
}

template<typename T>
size_t
Relayer<T>::get_n_elmts() const
{
    return this->n_elmts;
}

template<typename T>
void
Relayer<T>::set_ns(const size_t ns)
{
    this->ns = ns;
}

template<typename T>
void
Relayer<T>::relay(const T* in, T* out, const int frame_id, const bool managed_memory)
{
    (*this)[rly::sck::relay::in].bind(in);
    (*this)[rly::sck::relay::out].bind(out);
    (*this)[rly::tsk::relay].exec(frame_id, managed_memory);
}

template<typename T>
void
Relayer<T>::_relay(const T* in, T* out, const size_t frame_id)
{
    std::chrono::time_point<std::chrono::steady_clock> t_start;
    if (this->ns) t_start = std::chrono::steady_clock::now();

    if (in != out) std::copy(in, in + this->n_elmts, out);

    if (this->ns)
    {
        std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;
        while ((size_t)duration.count() < this->ns) // active waiting
            duration = std::chrono::steady_clock::now() - t_start;
    }
}

// ==================================================================================== explicit template instantiation
template class aff3ct::module::Relayer<int8_t>;
template class aff3ct::module::Relayer<uint8_t>;
template class aff3ct::module::Relayer<int16_t>;
template class aff3ct::module::Relayer<uint16_t>;
template class aff3ct::module::Relayer<int32_t>;
template class aff3ct::module::Relayer<uint32_t>;
template class aff3ct::module::Relayer<int64_t>;
template class aff3ct::module::Relayer<uint64_t>;
template class aff3ct::module::Relayer<float>;
template class aff3ct::module::Relayer<double>;
// ==================================================================================== explicit template instantiation
