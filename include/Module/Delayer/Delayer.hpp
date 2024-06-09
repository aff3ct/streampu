#ifndef DELAYER_HPP_
#define DELAYER_HPP_

#include <iostream>
#include <string>
#include <vector>

#include "Module/Module.hpp"

namespace spu
{
namespace module
{
namespace dly
{
enum class tsk : uint8_t
{
    memorize,
    produce,
    SIZE
};

namespace sck
{
enum class memorize : uint8_t
{
    in,
    status
};
enum class produce : uint8_t
{
    out,
    status
};
}
}

template<typename D = int>
class Delayer : public Module
{
  public:
    inline runtime::Task& operator[](const dly::tsk t);
    inline runtime::Socket& operator[](const dly::sck::memorize s);
    inline runtime::Socket& operator[](const dly::sck::produce s);
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t size; // Size of one frame (= number of datas in one frame)
    const D init_val;
    std::vector<D> data;

  public:
    Delayer(const size_t size, const D init_val);
    virtual ~Delayer() = default;
    virtual Delayer<D>* clone() const;
    virtual size_t get_size() const;
    virtual void set_n_frames(const size_t n_frames);
    void set_data(const std::vector<D>& init);
    void set_data(const D* init);

  protected:
    virtual void _memorize(const D* in, const size_t frame_id);
    virtual void _produce(D* out, const size_t frame_id);
};

}
}

#include "Module/Delayer/Delayer.hxx"

#endif /* DELAYER_HPP_ */
