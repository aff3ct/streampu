#include <algorithm>

#include "Module/Source/AZCW/Source_AZCW.hpp"

using namespace aff3ct::module;

template <typename B>
Source_AZCW<B>
::Source_AZCW(const int K)
: Source<B>(K)
{
	const std::string name = "Source_AZCW";
	this->set_name(name);
}

template <typename B>
Source_AZCW<B>* Source_AZCW<B>
::clone() const
{
	auto m = new Source_AZCW(*this);
	m->deep_copy(*this);
	return m;
}

template <typename B>
void Source_AZCW<B>
::_generate(B *U_K, const size_t frame_id)
{
	std::fill(U_K, U_K + this->K, 0);
}

// ==================================================================================== explicit template instantiation
template class aff3ct::module::Source_AZCW<int8_t>;
template class aff3ct::module::Source_AZCW<uint8_t>;
template class aff3ct::module::Source_AZCW<int16_t>;
template class aff3ct::module::Source_AZCW<uint16_t>;
template class aff3ct::module::Source_AZCW<int32_t>;
template class aff3ct::module::Source_AZCW<uint32_t>;
template class aff3ct::module::Source_AZCW<int64_t>;
template class aff3ct::module::Source_AZCW<uint64_t>;
template class aff3ct::module::Source_AZCW<float>;
template class aff3ct::module::Source_AZCW<double>;
// ==================================================================================== explicit template instantiation
