/*!
 * \file
 * \brief Class module::Subsequence.
 */
#ifndef SUBSEQUENCE_HPP_
#define SUBSEQUENCE_HPP_

#include <memory>
#include <cstddef>

#include "Module/Module.hpp"

namespace aff3ct
{
namespace runtime
{
class Sequence;
}
namespace module
{
	namespace ssq
	{
		enum class tsk : size_t { exec, SIZE };
	}

class Subsequence : public Module
{
	std::shared_ptr<runtime::Sequence> sequence_cloned;
	                runtime::Sequence *sequence_extern;

public:
	inline runtime::Task& operator[](const ssq::tsk t);

	explicit Subsequence(const runtime::Sequence &sequence);
	explicit Subsequence(      runtime::Sequence &sequence);
	virtual ~Subsequence() = default;

	virtual void init();

	virtual Subsequence* clone() const;

	runtime::Sequence& get_sequence();

	virtual void set_n_frames(const size_t n_frames);

protected:
	using Module::deep_copy;
	virtual void deep_copy(const Subsequence& m);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Subsequence/Subsequence.hxx"
#endif

#endif /* SUBSEQUENCE_HPP_ */
