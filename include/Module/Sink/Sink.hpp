/*!
 * \file
 * \brief Class module::Sink.
 */
#ifndef SINK_HPP
#define SINK_HPP

#include <cstdint>
#include <memory>
#include <vector>

#include "Tools/Interface/Interface_reset.hpp"
#include "Runtime/Task/Task.hpp"
#include "Runtime/Socket/Socket.hpp"
#include "Module/Module.hpp"

namespace aff3ct
{
namespace module
{
	namespace snk
	{
		enum class tsk : size_t { send, send_count, SIZE };

		namespace sck
		{
			enum class send : size_t { in_data, status };
			enum class send_count : size_t { in_data, in_count, status };
		}
	}

template <typename B = int>
class Sink : public Module, public tools::Interface_reset
{
public:
	inline runtime::Task&   operator[](const snk::tsk             t);
	inline runtime::Socket& operator[](const snk::sck::send       s);
	inline runtime::Socket& operator[](const snk::sck::send_count s);
	inline runtime::Socket& operator[](const std::string &tsk_sck  );

protected:
	const int max_data_size;

public:
	Sink(const int max_data_size);

	virtual ~Sink() = default;

	virtual Sink<B>* clone() const;

	template <class A = std::allocator<B>>
	void send(const std::vector<B,A>& in_data, const int frame_id = -1, const bool managed_memory = true);

	void send(const B *in_data, const int frame_id = -1, const bool managed_memory = true);

	template <class A = std::allocator<B>>
	void send_count(const std::vector<B,A>& in_data, const std::vector<uint32_t>& in_count, const int frame_id = -1,
	                const bool managed_memory = true);

	void send_count(const B *in_data, uint32_t *in_count, const int frame_id = -1, const bool managed_memory = true);

	virtual void reset();

protected:
	virtual void _send(const B *in_data, const size_t frame_id);
	virtual void _send_count(const B *in_data, const uint32_t *in_count, const size_t frame_id);
};

}
}

#include "Module/Sink/Sink.hxx"

#endif /* SINK_HPP */
