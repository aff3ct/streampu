/*!
 * \file
 * \brief Class module::Sink_NO.
 */
#ifndef SINK_NO_HPP
#define SINK_NO_HPP

#include "Module/Sink/Sink.hpp"

namespace spu
{
namespace module
{
/*!
 * \class Sink_NO
 *
 * \brief Send data to a binary file.
 *
 * \tparam B: type of the data to send or receive.
 *
 */
template<typename B = int>
class Sink_NO : public Sink<B>
{
  public:
    Sink_NO(const int max_data_size);
    ~Sink_NO() = default;

    virtual Sink_NO<B>* clone() const;

  protected:
    virtual void _send_count(const B* in_data, const uint32_t* in_count, const size_t frame_id);
};

}
}

#endif /* SINK_NO_HPP */
