/*!
 * \file
 * \brief Class module::Probe.
 */
#ifndef PROBE_HPP_
#define PROBE_HPP_

#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <typeindex>

#include "Runtime/Task/Task.hpp"
#include "Runtime/Socket/Socket.hpp"
#include "Module/Module.hpp"
#include "Tools/Interface/Interface_reset.hpp"
#include "Tools/Reporter/Probe/Reporter_probe.hpp"

namespace aff3ct
{
namespace module
{
	namespace prb
	{
		enum class tsk : uint8_t { probe, SIZE };

		namespace sck
		{
			enum class probe : uint8_t { in, status };
			enum class probe_noin : uint8_t { status };
		}
	}

class AProbe : public Module, public tools::Interface_reset
{
public:
	AProbe();
	virtual ~AProbe() = default;
	virtual void reset() = 0;
	virtual std::type_index get_datatype() const = 0;

	virtual void set_cname    (const std::string &name) = 0;
	virtual void set_unit     (const std::string &unit) = 0;
	virtual void set_buff_size(const size_t buffer_size) = 0;
	virtual void set_fmtflags (const std::ios_base::fmtflags ff) = 0;
	virtual void set_prec     (const size_t precision) = 0;
	virtual void set_col_size (const size_t col_size) = 0;
};

template <typename T = uint8_t>
class Probe : public AProbe
{
	friend tools::Reporter_probe;

public:
	inline runtime::Task&   operator[](const prb::tsk             t);
	inline runtime::Socket& operator[](const prb::sck::probe      s);
	inline runtime::Socket& operator[](const prb::sck::probe_noin s);
	inline runtime::Socket& operator[](const std::string &tsk_sck  );

protected:
	const int size;
	const std::string col_name;
	tools::Reporter_probe& reporter;
	Probe(const int size, const std::string &col_name, tools::Reporter_probe& reporter);

public:
	virtual ~Probe() = default;

	template <class AT = std::allocator<T>>
	void probe(const std::vector<T,AT>& in, const int frame_id = -1, const bool managed_memory = true);

	void probe(const T *in, const int frame_id = -1, const bool managed_memory = true);

	void probe(const int frame_id = -1, const bool managed_memory = true);

	virtual void reset();

	virtual void set_n_frames(const size_t n_frames);

	virtual void set_cname    (const std::string &name);
	virtual void set_unit     (const std::string &unit);
	virtual void set_buff_size(const size_t buffer_size);
	virtual void set_fmtflags (const std::ios_base::fmtflags ff);
	virtual void set_prec     (const size_t precision);
	virtual void set_col_size (const size_t col_size);

protected:
	virtual void _probe(const T *in, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Probe/Probe.hxx"
#endif

#endif /* PROBE_HPP_ */
