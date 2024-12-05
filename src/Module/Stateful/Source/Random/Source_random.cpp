#include "Module/Stateful/Source/Random/Source_random.hpp"

using namespace spu::module;

template<typename B>
Source_random<B>::Source_random(const int max_data_size, const int dec_granularity ,const int seed)
  : Source<B>(max_data_size)
  , dec_granularity(dec_granularity)
  , rd_engine(seed)
  , uniform_dist(0, 1)
{
    const std::string name = "Source_random";
    this->set_name(name);
    this->tasks[0]->set_replicability(true);
}

template<typename B>
Source_random<B>*
Source_random<B>::clone() const
{
    auto m = new Source_random(*this);
    m->deep_copy(*this);
    return m;
}

template<typename B>
void
Source_random<B>::_generate(B* out_data, const size_t frame_id)
{
    // generate a random k bits vector out_data
    for (auto i = 0; i < this->dec_granularity; i++)
        out_data[i] = (B)this->uniform_dist(this->rd_engine);
    
    // generate zero-padding out_data
    for (auto i = dec_granularity; i < this->max_data_size; i++)
        out_data[i] = (B)0;
}

template<typename B>
void
Source_random<B>::set_seed(const int seed)
{
    rd_engine.seed(seed);
}

// ==================================================================================== explicit template instantiation
template class spu::module::Source_random<int8_t>;
template class spu::module::Source_random<uint8_t>;
template class spu::module::Source_random<int16_t>;
template class spu::module::Source_random<uint16_t>;
template class spu::module::Source_random<int32_t>;
template class spu::module::Source_random<uint32_t>;
template class spu::module::Source_random<int64_t>;
template class spu::module::Source_random<uint64_t>;
template class spu::module::Source_random<float>;
template class spu::module::Source_random<double>;
// ==================================================================================== explicit template instantiation
