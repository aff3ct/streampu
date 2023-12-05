/*!
 * \file
 * \brief Class module::Binaryop.
 */
#ifndef BINARYOP_HPP_
#define BINARYOP_HPP_

#include <cstdint>
#include <memory>
#include <vector>

#include "Tools/Math/binaryop.h"
#include "Runtime/Task/Task.hpp"
#include "Runtime/Socket/Socket.hpp"
#include "Module/Module.hpp"

namespace aff3ct
{

namespace module
{
	namespace bop
	{
		enum class tsk : size_t { perform, fwd_perform, SIZE };

		namespace sck
		{
			enum class perform : size_t { in0, in1, out, status };
			enum class fwd_perform : size_t { in0, in1, status };
		}
	}

template <typename TI, typename TO, tools::proto_bop<TI,TO> BOP>
class Binaryop : public Module
{
public:
	inline runtime::Task&   operator[](const bop::tsk              t      );
	inline runtime::Socket& operator[](const bop::sck::perform     s      );
	inline runtime::Socket& operator[](const bop::sck::fwd_perform s      );
	inline runtime::Socket& operator[](const std::string&          tsk_sck);

protected:
	const size_t n_elmts;

public:
	Binaryop(const size_t n_in1, const size_t n_in2);
	Binaryop(const size_t n_elmts);
	virtual ~Binaryop() = default;
	virtual Binaryop<TI,TO,BOP>* clone() const;

	size_t get_n_elmts() const;

	template <class AI = std::allocator<TI>, class AO = std::allocator<TO>>
	void perform(const std::vector<TI,AI>& in1,
	             const std::vector<TI,AI>& in2,
	                   std::vector<TO,AO>& out,
	             const int frame_id = -1,
	             const bool managed_memory = true);

	void perform(const TI *in1, const TI *in2, TO *out, const int frame_id = -1, const bool managed_memory = true);

protected:
	virtual void _perform(const TI *in1, const TI *in2, TO *out, const size_t frame_id);
	virtual void _perform(const TI  in1, const TI *in2, TO *out, const size_t frame_id);
	virtual void _perform(const TI *in1, const TI  in2, TO *out, const size_t frame_id);
	virtual void _perform(      TI *in1, const TI *in2,          const size_t frame_id);
	virtual void _perform(      TI *in1, const TI  in2,          const size_t frame_id);
};

template <typename TI, typename TO = TI> using Binaryop_add = Binaryop<TI,TO,tools::bop_add<TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_sub = Binaryop<TI,TO,tools::bop_sub<TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_mul = Binaryop<TI,TO,tools::bop_mul<TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_div = Binaryop<TI,TO,tools::bop_div<TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_or  = Binaryop<TI,TO,tools::bop_or <TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_xor = Binaryop<TI,TO,tools::bop_xor<TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_and = Binaryop<TI,TO,tools::bop_and<TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_min = Binaryop<TI,TO,tools::bop_min<TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_max = Binaryop<TI,TO,tools::bop_max<TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_gt  = Binaryop<TI,TO,tools::bop_gt <TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_ge  = Binaryop<TI,TO,tools::bop_ge <TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_lt  = Binaryop<TI,TO,tools::bop_lt <TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_le  = Binaryop<TI,TO,tools::bop_le <TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_ne  = Binaryop<TI,TO,tools::bop_ne <TI,TO>>;
template <typename TI, typename TO = TI> using Binaryop_eq  = Binaryop<TI,TO,tools::bop_eq <TI,TO>>;

}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Binaryop/Binaryop.hxx"
#endif

#endif /* BINARYOP_HPP_ */
