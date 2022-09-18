#include "Runtime/Pipeline/Pipeline.hpp"

namespace aff3ct
{
namespace runtime
{

template <class C>
std::vector<C*> Pipeline
::get_modules(const bool subsequence_modules) const
{
	std::vector<C*> ret;
	for (auto &stage : this->stages)
	{
		auto modules = stage->get_modules<C>(subsequence_modules);
		ret.insert(ret.end(), modules.begin(), modules.end());
	}
	return ret;
}

}
}
