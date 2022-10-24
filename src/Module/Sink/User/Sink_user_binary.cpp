#include <sstream>
#include <cstdint>
#include <ios>

#include "Tools/Algo/Bit_packer/Bit_packer.hpp"
#include "Tools/Exception/exception.hpp"
#include "Module/Sink/User/Sink_user_binary.hpp"

using namespace aff3ct;
using namespace aff3ct::module;

template <typename B>
Sink_user_binary<B>
::Sink_user_binary(const int max_data_size, const std::string &filename)
: Sink<B>(max_data_size),
  filename(filename),
  sink_file(filename.c_str(), std::ios::out | std::ios::binary),
  chunk(max_data_size),
  reconstructed_buffer(CHAR_BIT),
  n_left(0)
{
	const std::string name = "Sink_user_binary";
	this->set_name(name);

	if (this->max_data_size < CHAR_BIT)
	{
		std::stringstream message;
		message << "'max_data_size' has to be greater or equal to 'CHAR_BIT' ('max_data_size' = " <<  this->max_data_size
		        << ", 'CHAR_BIT' = " <<  CHAR_BIT << ").";
		throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
	}

	if (sink_file.fail())
	{
		std::stringstream message;
		message << "'filename' file name is not valid: sink file failbit is set.";
		throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
	}
}

template <typename B>
void Sink_user_binary<B>
::reset()
{
	sink_file.close();
	sink_file.open(this->filename.c_str(), std::ios::out | std::ios::binary);
	if (sink_file.fail())
		throw tools::runtime_error(__FILE__, __LINE__, __func__, "Could not go back to the beginning of the file.");
	this->n_left = 0;
}

template <typename B>
void Sink_user_binary<B>
::_send_count(const B *in_data, const uint32_t *in_count, const size_t frame_id)
{
	size_t n_completing  = (CHAR_BIT - this->n_left) % CHAR_BIT; // number of bits that are needed to complete one byte
	char reconstructed_byte;                                     // to store reconstructed byte (n_left & n_completing)

	if (sink_file.fail())
	{
		std::stringstream message;
		message << "'filename' file name is not valid: sink file failbit is set.";
		throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
	}

	if (this->n_left != 0)
	{
		for (size_t i = 0; i < n_completing; i++) // completing byte with n_completing first bits of in_data
			this->reconstructed_buffer[this->n_left +i] = in_data[i];

		tools::Bit_packer::pack(this->reconstructed_buffer.data(), &reconstructed_byte, CHAR_BIT);
		sink_file.write(&reconstructed_byte, 1);
	}

	size_t main_chunk_size = (*in_count - n_completing) / CHAR_BIT; // in byte
	this->n_left           = (*in_count - n_completing) % CHAR_BIT;

	tools::Bit_packer::pack(in_data + n_completing, this->chunk.data(), main_chunk_size * CHAR_BIT);
	sink_file.write(this->chunk.data(), main_chunk_size);
	sink_file.flush();
	this->n_left = 0;
	for (size_t i = n_completing + main_chunk_size * CHAR_BIT; i < (size_t)*in_count; i++)
		this->reconstructed_buffer[this->n_left++] = in_data[i];
}

// ==================================================================================== explicit template instantiation
template class aff3ct::module::Sink_user_binary<int8_t>;
template class aff3ct::module::Sink_user_binary<uint8_t>;
template class aff3ct::module::Sink_user_binary<int16_t>;
template class aff3ct::module::Sink_user_binary<uint16_t>;
template class aff3ct::module::Sink_user_binary<int32_t>;
template class aff3ct::module::Sink_user_binary<uint32_t>;
template class aff3ct::module::Sink_user_binary<int64_t>;
template class aff3ct::module::Sink_user_binary<uint64_t>;
template class aff3ct::module::Sink_user_binary<float>;
template class aff3ct::module::Sink_user_binary<double>;
// ==================================================================================== explicit template instantiation
