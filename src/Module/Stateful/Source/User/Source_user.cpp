#include <fstream>
#include <sstream>

#include "Module/Stateful/Source/User/Source_user.hpp"
#include "Tools/Exception/exception.hpp"

using namespace spu;
using namespace spu::module;

template<typename B>
Source_user<B>::Source_user(const int max_data_size,
                            const std::string& filename,
                            const bool auto_reset,
                            const int start_idx)
  : Source<B>(max_data_size)
  , source()
  , next_frame_idx(start_idx)
  , src_counter(0)
  , auto_reset(auto_reset)
  , start_idx(start_idx)
  , done(false)
{
    const std::string name = "Source_user";
    this->set_name(name);

    if (filename.empty())
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, "'filename' should not be empty.");

    std::ifstream file(filename.c_str(), std::ios::in);

    if (file.is_open())
    {
        int n_src = 0, src_size = 0;

        file >> n_src;
        file >> src_size;

        if (n_src <= 0 || src_size <= 0)
        {
            std::stringstream message;
            message << "'n_src', and 'src_size' have to be greater than 0 ('n_src' = " << n_src
                    << ", 'src_size' = " << src_size << ").";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

        this->source.resize(n_src);
        for (auto i = 0; i < n_src; i++)
            this->source[i].resize(src_size);

        if (src_size == this->max_data_size)
        {
            for (auto i = 0; i < n_src; i++)
                for (auto j = 0; j < src_size; j++)
                {
                    int bit;
                    file >> bit;

                    this->source[i][j] = bit != 0;
                }
        }
        else
        {
            file.close();

            std::stringstream message;
            message << "The size is wrong (read: " << src_size << ", expected: " << this->max_data_size << ").";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

        file.close();
    }
    else
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, "Can't open '" + filename + "' file.");

    this->next_frame_idx %= (int)source.size();
}

template<typename B>
Source_user<B>::Source_user(const int max_data_size, const std::string& filename, const int start_idx)
  : Source_user<B>(max_data_size, filename, true, start_idx)
{
}

template<typename B>
Source_user<B>*
Source_user<B>::clone() const
{
    auto m = new Source_user(*this);
    m->deep_copy(*this);
    return m;
}

template<typename B>
void
Source_user<B>::reset()
{
    this->next_frame_idx = this->start_idx;
    this->src_counter = 0;
    this->done = false;
}

template<typename B>
bool
Source_user<B>::is_done() const
{
    return this->done;
}

template<typename B>
void
Source_user<B>::_generate(B* out_data, const size_t frame_id)
{
    std::copy(this->source[this->next_frame_idx].begin(), this->source[this->next_frame_idx].end(), out_data);

    this->next_frame_idx = (this->next_frame_idx + 1) % (int)this->source.size();

    if (this->auto_reset == false)
    {
        this->src_counter = (this->src_counter + 1) % (int)this->source.size();
        if (this->src_counter == 0) this->done = true;
    }
}

// ==================================================================================== explicit template instantiation
template class spu::module::Source_user<int8_t>;
template class spu::module::Source_user<uint8_t>;
template class spu::module::Source_user<int16_t>;
template class spu::module::Source_user<uint16_t>;
template class spu::module::Source_user<int32_t>;
template class spu::module::Source_user<uint32_t>;
template class spu::module::Source_user<int64_t>;
template class spu::module::Source_user<uint64_t>;
template class spu::module::Source_user<float>;
template class spu::module::Source_user<double>;
// ==================================================================================== explicit template instantiation
