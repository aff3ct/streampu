/*!
 * \file
 * \brief Class module::Relayer_fwd.
 */
#ifndef RELAYERFWD_HPP_
#define RELAYERFWD_HPP_

#include <cstdint>
#include <vector>

#include "Module/Module.hpp"

namespace aff3ct
{
namespace module
{
	namespace rly_fwd
	{
		enum class tsk : size_t { relay_fwd, SIZE };

		namespace sck
		{
			enum class relay_fwd : size_t { fwd, status };
		}
	}

template <typename T = int>
class Relayer_fwd : public Module
{
public:
	inline runtime::Task&   operator[](const rly_fwd::tsk        t);
	inline runtime::Socket& operator[](const rly_fwd::sck::relay_fwd s);

protected:
	const size_t n_elmts;
	size_t ns;

public:
	Relayer_fwd(const size_t n_elmts, const size_t ns = 0);
	virtual ~Relayer_fwd() = default;
	virtual Relayer_fwd* clone() const;

	size_t get_ns() const;
	void set_ns(const size_t ns);
	size_t get_n_elmts() const;

	template <class A = std::allocator<T>>
	void relay_fwd(std::vector<T,A>& fwd,
	               const int frame_id = -1,
	               const bool managed_memory = true);

	void relay_fwd(T *fwd, const int frame_id = -1, const bool managed_memory = true);

protected:
	virtual void _relay_fwd(T *fwd, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Relayer/Relayer_fwd.hxx"
#endif

#endif /* RELAYERFWD_HPP_ */
