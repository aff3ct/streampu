/*!
 * \file
 * \brief Class tools::Reporter_probe.
 */
#ifndef REPORTER_PROBE_HPP_
#define REPORTER_PROBE_HPP_

#include <initializer_list>
#include <typeindex>
#include <iostream>
#include <cstddef>
#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <ios>

#include "Tools/Reporter/Reporter.hpp"

namespace aff3ct
{
namespace module
{
	class AProbe;
}
namespace tools
{

class Reporter_probe : public Reporter
{
protected:
	std::vector<module::AProbe*> probes;

	std::vector<size_t> head;
	std::vector<size_t> tail;
	std::vector<std::vector<std::vector<int8_t>>> buffer;
	std::vector<std::type_index> datatypes;
	std::vector<std::ios_base::fmtflags> format_flags;
	std::vector<size_t> precisions;
	std::vector<size_t> column_sizes;
	std::vector<size_t> data_sizes;
	std::map<std::string, int> name_to_col;
	std::map<int, std::string> col_to_name;
	size_t n_frames;
	std::vector<std::mutex> mtx;

public:
	Reporter_probe(const std::string &group_name, const std::string &group_description);

	explicit Reporter_probe(const std::string &group_name);

	virtual ~Reporter_probe() = default;

	virtual report_t report(bool final = false);

	virtual void reset();

	void set_n_frames(const size_t n_frames);

	size_t get_n_frames() const;

	void set_col_unit(const std::string &unit, const module::AProbe &prb);
	void set_col_buff_size(const size_t buffer_size, const module::AProbe &prb);
	void set_col_fmtflags(const std::ios_base::fmtflags ff, const module::AProbe &prb);
	void set_col_prec(const size_t precision, const module::AProbe &prb);
	void set_col_size(const size_t col_size, const module::AProbe &prb);

	void set_cols_unit(const std::string &unit);
	void set_cols_buff_size(const size_t buffer_size);
	void set_cols_fmtflags(const std::ios_base::fmtflags ff);
	void set_cols_prec(const size_t precision);
	void set_cols_size(const size_t col_size);

	void register_probe(module::AProbe* probe, const size_t data_size, const std::type_index data_type,
	                    const std::string &unit, const size_t buffer_size, const std::ios_base::fmtflags ff,
	                    const size_t precision);

	virtual void probe(const std::string &name, const void *data, const size_t frame_id);

	void register_probes(const std::initializer_list<module::AProbe*> &probes);

protected:
	void create_probe_checks(const std::string &name);

	template <typename T>
	size_t col_size(const int col);

	template <typename T>
	bool push(const int col, const T *data);

	template <typename T>
	bool pull(const int col, T *data);

	size_t get_probe_index(const module::AProbe &prb);

private:
	template <typename T>
	bool format_values(const int col, std::stringstream &temp_stream);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Tools/Reporter/Probe/Reporter_probe.hxx"
#endif

#endif /* REPORTER_PROBE_HPP_ */
