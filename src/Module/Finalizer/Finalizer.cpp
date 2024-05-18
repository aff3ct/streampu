#include <sstream>
#include <string>

#include "Module/Finalizer/Finalizer.hpp"

using namespace spu;
using namespace spu::module;

template<typename T>
Finalizer<T>::Finalizer(const size_t n_elmts, const size_t history_size)
  : Module()
  , data(history_size, std::vector<std::vector<T>>(this->get_n_frames(), std::vector<T>(n_elmts, 0)))
  , next_stream_id(0)
{
    const std::string name = "Finalizer";
    this->set_name(name);
    this->set_short_name(name);

    if (n_elmts == 0)
    {
        std::stringstream message;
        message << "'n_elmts' has to be greater than 0.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (history_size == 0)
    {
        std::stringstream message;
        message << "'history_size' has to be greater than 0.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    auto& p = this->create_task("finalize");
    auto ps_in = this->template create_socket_in<T>(p, "in", n_elmts);
    this->create_codelet(p,
                         [ps_in](Module& m, runtime::Task& t, const size_t frame_id) -> int
                         {
                             auto& fin = static_cast<Finalizer&>(m);
                             fin._finalize(t[ps_in].template get_dataptr<const T>(), frame_id);
                             return runtime::status_t::SUCCESS;
                         });
}

template<typename T>
Finalizer<T>*
Finalizer<T>::clone() const
{
    auto m = new Finalizer(*this);
    m->deep_copy(*this);
    return m;
}

template<typename T>
const std::vector<std::vector<T>>&
Finalizer<T>::get_final_data() const
{
    auto last_stream_id = this->next_stream_id ? this->next_stream_id - 1 : this->data.size() - 1;
    return this->data[last_stream_id];
}

template<typename T>
const std::vector<std::vector<std::vector<T>>>&
Finalizer<T>::get_histo_data() const
{
    return this->data;
}

template<typename T>
size_t
Finalizer<T>::get_next_stream_id() const
{
    return this->next_stream_id;
}

template<typename T>
void
Finalizer<T>::set_n_frames(const size_t n_frames)
{
    const auto old_n_frames = this->get_n_frames();
    if (old_n_frames != n_frames)
    {
        Module::set_n_frames(n_frames);
        for (size_t s = 0; s < this->data.size(); s++)
        {
            this->data[s].resize(n_frames);
            for (size_t f = old_n_frames; f < n_frames; f++)
                this->data[s][f].resize(this->data[s][0].size());
        }
    }
}

template<typename T>
void
Finalizer<T>::finalize(const T* in, const int frame_id, const bool managed_memory)
{
    (*this)[fin::sck::finalize::in].bind(in);
    (*this)[fin::tsk::finalize].exec(frame_id, managed_memory);
}

template<typename T>
void
Finalizer<T>::_finalize(const T* in, const size_t frame_id)
{
    std::copy(in, in + this->data[this->next_stream_id][0].size(), this->data[this->next_stream_id][frame_id].begin());

    if (frame_id == this->get_n_frames() - 1) this->next_stream_id = (this->next_stream_id + 1) % this->data.size();
}

template<typename T>
void
Finalizer<T>::reset()
{
    this->next_stream_id = 0;
}

// ==================================================================================== explicit template instantiation
template class spu::module::Finalizer<int8_t>;
template class spu::module::Finalizer<uint8_t>;
template class spu::module::Finalizer<int16_t>;
template class spu::module::Finalizer<uint16_t>;
template class spu::module::Finalizer<int32_t>;
template class spu::module::Finalizer<uint32_t>;
template class spu::module::Finalizer<int64_t>;
template class spu::module::Finalizer<uint64_t>;
template class spu::module::Finalizer<float>;
template class spu::module::Finalizer<double>;
// ==================================================================================== explicit template instantiation
