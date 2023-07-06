/*!
 * \file
 * \brief Class module::Relayer_io.
 */
#ifndef RELAYERIO_HPP_
#define RELAYERIO_HPP_

#include <cstdint>
#include <vector>

#include "Module/Module.hpp"

namespace aff3ct
{
namespace module
{
	namespace rly_io
	{
		enum class tsk : size_t { relay_io, SIZE };

		namespace sck
		{
			enum class relay_io : size_t { inout, status };
		}
	}

template <typename T = int>
class Relayer_io : public Module
{
public:
	inline runtime::Task&   operator[](const rly_io::tsk        t);
	inline runtime::Socket& operator[](const rly_io::sck::relay_io s);

protected:
	const size_t n_elmts;
	size_t ns;

public:
	Relayer_io(const size_t n_elmts, const size_t ns = 0);
	virtual ~Relayer_io() = default;
	virtual Relayer_io* clone() const;

	size_t get_ns() const;
	void set_ns(const size_t ns);
	size_t get_n_elmts() const;

	template <class A = std::allocator<T>>
	void relay_io(std::vector<T,A>& inout,
	           const int frame_id = -1,
	           const bool managed_memory = true);

	void relay_io(T *inout, const int frame_id = -1, const bool managed_memory = true);

protected:
	virtual void _relay_io(T *inout, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Relayer_io/Relayer_io.hxx"
#endif

#endif /* RELAYERIO_HPP_ */
