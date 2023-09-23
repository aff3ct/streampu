/*!
 * \file
 * \brief Class module::Incrementer_fwd.
 */

#ifndef INCREMENTERFWD_HPP_
#define INCREMENTERFWD_HPP_

#include <cstdint>
#include <vector>

#include "Module/Module.hpp"

namespace aff3ct
{
namespace module
{
	namespace inc_fwd
	{
		enum class tsk : size_t { increment_fwd, SIZE };

		namespace sck
		{
			enum class increment_fwd : size_t { fwd, status };
		}
	}

template <typename T = int>
class Incrementer_fwd : public Module
{
public:
	inline runtime::Task&   operator[](const inc_fwd::tsk                t);
	inline runtime::Socket& operator[](const inc_fwd::sck::increment_fwd s);
	inline runtime::Socket& operator[](const std::string &tsk_sck         );

protected:
	const size_t n_elmts;
	size_t ns;

public:
	Incrementer_fwd(const size_t n_elmts, const size_t ns = 0);
	virtual ~Incrementer_fwd() = default;
	virtual Incrementer_fwd* clone() const;

	size_t get_ns() const;
	void set_ns(const size_t ns);
	size_t get_n_elmts() const;

	template <class A = std::allocator<T>>
	void increment_fwd(std::vector<T,A>& fwd,
	               const int frame_id = -1,
	               const bool managed_memory = true);

	void increment_fwd(T *fwd, const int frame_id = -1, const bool managed_memory = true);


protected:
	virtual void _increment_fwd(T *fwd, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Incrementer/Incrementer_fwd.hxx"
#endif

#endif /* INCREMENTERFWD_HPP_ */
