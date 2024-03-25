/*!
 * \file
 * \brief Class tools::Reporter_probe.
 */
#ifndef REPORTER_PROBE_HPP_
#define REPORTER_PROBE_HPP_

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
	template <typename T> class Probe_value;
	class Probe_throughput;
	class Probe_latency;
	class Probe_time;
	class Probe_timestamp;
	class Probe_occurrence;
	class Probe_stream;
}
namespace tools
{

class Reporter_probe : public Reporter
{
protected:
	std::vector<std::unique_ptr<module::AProbe>> probes;

	std::vector<size_t> head;
	std::vector<size_t> tail;
	std::vector<std::vector<std::vector<int8_t>>> buffer;
	std::vector<std::type_index> datatypes;
	std::vector<std::ios_base::fmtflags> stream_flags;
	std::vector<size_t> precisions;
	std::vector<size_t> datasizes;
	std::map<std::string, int> name_to_col;
	std::map<int, std::string> col_to_name;
	size_t n_frames;
	Reporter::report_t final_report;
	std::vector<std::mutex> mtx;

public:
	Reporter_probe(const std::string &group_name, const std::string &group_description);

	explicit Reporter_probe(const std::string &group_name);

	virtual ~Reporter_probe() = default;

	virtual report_t report(bool final = false);

	template <typename T>
	module::Probe_value<T>& create_probe_value(const std::string &name,
	                                           const std::string &unit = "",
	                                           const size_t buffer_size = 100,
	                                           const size_t socket_size = 1,
	                                           const std::ios_base::fmtflags ff = std::ios_base::scientific,
	                                           const size_t precision = 3);

	module::Probe_throughput& create_probe_throughput_mbps(const std::string &name,
	                                                       const size_t data_size = 1,
	                                                       const size_t buffer_size = 100,
	                                                       const std::ios_base::fmtflags ff = std::ios_base::dec | std::ios_base::fixed,
	                                                       const size_t precision = 3);

	module::Probe_throughput& create_probe_throughput(const std::string &name,
	                                                  const std::string &unit = "",
	                                                  const size_t data_size = 1,
	                                                  const double factor = 1.,
	                                                  const size_t buffer_size = 100,
	                                                  const std::ios_base::fmtflags ff = std::ios_base::dec | std::ios_base::fixed,
	                                                  const size_t precision = 3);

	module::Probe_latency& create_probe_latency(const std::string &name,
	                                            const size_t buffer_size = 100,
	                                            const std::ios_base::fmtflags ff = std::ios_base::scientific,
	                                            const size_t precision = 3);

	module::Probe_time& create_probe_time(const std::string &name,
	                                      const size_t buffer_size = 100,
	                                      const std::ios_base::fmtflags ff = std::ios_base::dec | std::ios_base::fixed,
	                                      const size_t precision = 2);

	module::Probe_timestamp& create_probe_timestamp(const std::string &name,
	                                                const size_t buffer_size = 100,
	                                                const std::ios_base::fmtflags ff = std::ios_base::scientific,
	                                                const size_t precision = 2);

	module::Probe_timestamp& create_probe_timestamp_mod(const std::string &name,
	                                                    const uint64_t mod,
	                                                    const size_t buffer_size = 100,
	                                                    const std::ios_base::fmtflags ff = std::ios_base::scientific,
	                                                    const size_t precision = 2);

	module::Probe_occurrence& create_probe_occurrence(const std::string &name,
	                                                  const std::string &unit = "",
	                                                  const size_t buffer_size = 100,
	                                                  const std::ios_base::fmtflags ff = std::ios_base::scientific,
	                                                  const size_t precision = 3);

	module::Probe_stream& create_probe_stream(const std::string &name,
	                                          const std::string &unit = "",
	                                          const size_t buffer_size = 100,
	                                          const std::ios_base::fmtflags ff = std::ios_base::scientific,
	                                          const size_t precision = 3);

	virtual void probe(const std::string &name, const void *data, const size_t frame_id);

	virtual void reset();

	void set_n_frames(const size_t n_frames);

	size_t get_n_frames() const;

protected:
	void create_probe_checks(const std::string &name);

	template <typename T>
	size_t col_size(const int col);

	template <typename T>
	bool push(const int col, const T *data);

	template <typename T>
	bool pull(const int col, T *data);

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
