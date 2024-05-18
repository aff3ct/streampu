#include "Tools/Reporter/Reporter.hpp"

using namespace spu;
using namespace spu::tools;

void
Reporter::init()
{
}

const std::vector<Reporter::group_t>&
Reporter::get_groups() const
{
    return this->cols_groups;
}

void
Reporter::reset()
{
}