/*!
 * \file
 * \brief Class module::Sink_user_binary.
 */
#ifndef SINK_USER_BINARY_HPP
#define SINK_USER_BINARY_HPP

#include <fstream>
#include <cstddef>
#include <string>
#include <vector>

#include "Module/Sink/Sink.hpp"

namespace aff3ct
{
namespace module
{
/*!
 * \class Sink_user_binary
 *
 * \brief Send data to a binary file.
 *
 * \tparam B: type of the data to send or receive.
 *
 */
template <typename B = int>
class Sink_user_binary : public Sink<B>
{
private:
	const std::string filename;
	std::ofstream sink_file;
	std::vector<char> chunk;
	std::vector<B> reconstructed_buffer;
	size_t n_left; // number of bits left by last call

public:
	Sink_user_binary(const int max_data_size, const std::string &filename);
	~Sink_user_binary() = default;

	virtual void reset();

protected:
	virtual void _send_count(const B *in_data, const uint32_t *in_count, const size_t frame_id);
};

}
}

#endif /* SINK_USER_BINARY_HPP */
