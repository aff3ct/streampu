/*!
 * \file
 * \brief Class module::Incrementer_io.
 */

#ifndef INCREMENTERIO_HPP_
#define INCREMENTERIO_HPP_

#include <cstdint>
#include <vector>

#include "Module/Module.hpp"

namespace aff3ct
{
namespace module
{
	namespace inc_io
	{
		enum class tsk : size_t { increment_io, SIZE };

		namespace sck
		{
			enum class increment_io : size_t {inout,status };
		}
	}

template <typename T = int>
class Incrementer_io : public Module
{
public:
	inline runtime::Task&   operator[](const inc_io::tsk            t);
	inline runtime::Socket& operator[](const inc_io::sck::increment_io s);

protected:
	const size_t n_elmts;
	size_t ns;

public:
	Incrementer_io(const size_t n_elmts, const size_t ns = 0);
	virtual ~Incrementer_io() = default;
	virtual Incrementer_io* clone() const;

	size_t get_ns() const;
	void set_ns(const size_t ns);
	size_t get_n_elmts() const;

	template <class A = std::allocator<T>>
	void increment_io(std::vector<T,A>& inout,
	               const int frame_id = -1,
	               const bool managed_memory = true);

	void increment_io(T *inout, const int frame_id = -1, const bool managed_memory = true);


protected:
	virtual void _increment_io(T *inout, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Incrementer_io/Incrementer_io.hxx"
#endif

#endif /* INCREMENTERIO_HPP_ */
