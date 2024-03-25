#include "Module/Set/Set.hpp"

namespace aff3ct
{
namespace module
{

runtime::Task& Set
::operator[](const set::tsk t)
{
	return Module::operator[]((size_t)t);
}

}
}