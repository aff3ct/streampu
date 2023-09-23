/*!
 * \file
 * \brief Class module::Relayer.
 */
#ifndef RELAYER_HPP_
#define RELAYER_HPP_

#include <cstdint>
#include <vector>

#include "Module/Module.hpp"

namespace aff3ct
{
namespace module
{
	namespace rly
	{
		enum class tsk : size_t { relay, SIZE };

		namespace sck
		{
			enum class relay : size_t { in, out, status };
		}
	}

template <typename T = int>
class Relayer : public Module
{
public:
	inline runtime::Task&   operator[](const rly::tsk           t);
	inline runtime::Socket& operator[](const rly::sck::relay    s);
	inline runtime::Socket& operator[](const std::string &tsk_sck);

protected:
	const size_t n_elmts;
	size_t ns;

public:
	Relayer(const size_t n_elmts, const size_t ns = 0);
	virtual ~Relayer() = default;
	virtual Relayer* clone() const;

	size_t get_ns() const;
	void set_ns(const size_t ns);
	size_t get_n_elmts() const;

	template <class A = std::allocator<T>>
	void relay(const std::vector<T,A>& in,
	                 std::vector<T,A>& out,
	           const int frame_id = -1,
	           const bool managed_memory = true);

	void relay(const T *in, T *out, const int frame_id = -1, const bool managed_memory = true);

protected:
	virtual void _relay(const T *in, T *out, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Relayer/Relayer.hxx"
#endif

#endif /* RELAYER_HPP_ */
