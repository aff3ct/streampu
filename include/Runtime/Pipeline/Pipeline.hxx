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

const std::vector<std::vector<runtime::Task*>>& Pipeline
::get_firsts_tasks() const
{
	return this->stages[0]->get_firsts_tasks();
}

const std::vector<std::vector<runtime::Task*>>& Pipeline
::get_lasts_tasks() const
{
	return this->stages.back()->get_lasts_tasks();
}
}
}
