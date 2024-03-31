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
protected:
	tools::Reporter_probe* reporter;

public:
	AProbe();
	virtual ~AProbe() = default;
	virtual void reset() = 0;

	virtual void set_col_unit     (const std::string &unit) = 0;
	virtual void set_col_buff_size(const size_t buffer_size) = 0;
	virtual void set_col_fmtflags (const std::ios_base::fmtflags ff) = 0;
	virtual void set_col_prec     (const size_t precision) = 0;
	virtual void set_col_size     (const size_t col_size) = 0;

	virtual void register_reporter(tools::Reporter_probe* reporter) = 0;

	virtual const std::string& get_col_name() const = 0;

	inline runtime::Task&   operator[](const prb::tsk             t);
	inline runtime::Socket& operator[](const prb::sck::probe      s);
	inline runtime::Socket& operator[](const prb::sck::probe_noin s);
	inline runtime::Socket& operator[](const std::string   &tsk_sck);

protected:
	void check_reporter();
	void proxy_register_probe(const size_t data_size, const std::type_index data_type, const std::string &unit,
	                          const size_t buffer_size, const std::ios_base::fmtflags ff, const size_t precision);
	void proxy_probe(const void *data, const size_t frame_id);
};

template <typename T = uint8_t>
class Probe : public AProbe
{
public:
	inline runtime::Task&   operator[](const prb::tsk             t);
	inline runtime::Socket& operator[](const prb::sck::probe      s);
	inline runtime::Socket& operator[](const prb::sck::probe_noin s);
	inline runtime::Socket& operator[](const std::string   &tsk_sck);

protected:
	const size_t socket_size;
	const std::string col_name;
	Probe(const size_t socket_size, const std::string &col_name);

public:
	virtual ~Probe() = default;

	template <class AT = std::allocator<T>>
	void probe(const std::vector<T,AT>& in, const int frame_id = -1, const bool managed_memory = true);

	void probe(const T *in, const int frame_id = -1, const bool managed_memory = true);

	void probe(const int frame_id = -1, const bool managed_memory = true);

	virtual void reset();

	virtual void set_n_frames(const size_t n_frames);

	virtual void set_col_unit     (const std::string &unit);
	virtual void set_col_buff_size(const size_t buffer_size);
	virtual void set_col_fmtflags (const std::ios_base::fmtflags ff);
	virtual void set_col_prec     (const size_t precision);
	virtual void set_col_size     (const size_t col_size);

	const std::string& get_col_name() const;
	const size_t get_socket_size() const;

protected:
	virtual void _probe(const T *in, const size_t frame_id);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Probe/Probe.hxx"
#endif

#endif /* PROBE_HPP_ */
