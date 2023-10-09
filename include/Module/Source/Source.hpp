/*!
 * \file
 * \brief Class module::Source.
 */
#ifndef SOURCE_HPP_
#define SOURCE_HPP_

#include <cstdint>
#include <memory>
#include <vector>

#include "Tools/Interface/Interface_set_seed.hpp"
#include "Tools/Interface/Interface_is_done.hpp"
#include "Tools/Interface/Interface_reset.hpp"
#include "Runtime/Task/Task.hpp"
#include "Runtime/Socket/Socket.hpp"
#include "Module/Module.hpp"

namespace aff3ct
{
namespace module
{
	namespace src
	{
		enum class tsk : size_t { generate, SIZE };

		namespace sck
		{
			enum class generate : size_t { out_data, out_count, status };
		}
	}

template <typename B = int>
class Source : public Module, public tools::Interface_set_seed,
                              public tools::Interface_is_done,
                              public tools::Interface_reset
{
public:
	inline runtime::Task&   operator[](const src::tsk           t);
	inline runtime::Socket& operator[](const src::sck::generate s);
	inline runtime::Socket& operator[](const std::string &tsk_sck);

protected:
	const int max_data_size;

public:
	Source(const int max_data_size);

	virtual ~Source() = default;

	virtual Source<B>* clone() const;

	virtual int get_max_data_size() const;

	template <class A = std::allocator<B>>
	void generate(std::vector<B,A>& out_data, const int frame_id = -1, const bool managed_memory = true);

	void generate(B *out_data, const int frame_id = -1, const bool managed_memory = true);

	template <class A = std::allocator<B>>
	void generate(std::vector<B,A>& out_data, std::vector<uint32_t>& out_count, const int frame_id = -1,
	              const bool managed_memory = true);

	void generate(B *out_data, uint32_t *out_count, const int frame_id = -1, const bool managed_memory = true);

	virtual void set_seed(const int seed);

	virtual bool is_done() const;

	virtual void reset();

protected:
	virtual void _generate(B *out_data, const size_t frame_id);
	virtual void _generate(B *out_data, uint32_t *out_count, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Source/Source.hxx"
#endif

#endif /* SOURCE_HPP_ */
