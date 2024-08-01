#include <algorithm>

#include "Module/Stateful/Source/AZCW/Source_AZCW.hpp"

using namespace spu::module;

template<typename B>
Source_AZCW<B>::Source_AZCW(const int max_data_size)
  : Source<B>(max_data_size)
{
    const std::string name = "Source_AZCW";
    this->set_name(name);
    this->tasks[0]->set_replicability(true);
}

template<typename B>
Source_AZCW<B>*
Source_AZCW<B>::clone() const
{
    auto m = new Source_AZCW(*this);
    m->deep_copy(*this);
    return m;
}

template<typename B>
void
Source_AZCW<B>::_generate(B* out_data, const size_t frame_id)
{
    std::fill(out_data, out_data + this->max_data_size, 0);
}

// ==================================================================================== explicit template instantiation
template class spu::module::Source_AZCW<int8_t>;
template class spu::module::Source_AZCW<uint8_t>;
template class spu::module::Source_AZCW<int16_t>;
template class spu::module::Source_AZCW<uint16_t>;
template class spu::module::Source_AZCW<int32_t>;
template class spu::module::Source_AZCW<uint32_t>;
template class spu::module::Source_AZCW<int64_t>;
template class spu::module::Source_AZCW<uint64_t>;
template class spu::module::Source_AZCW<float>;
template class spu::module::Source_AZCW<double>;
// ==================================================================================== explicit template instantiation
