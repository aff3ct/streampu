#include <sstream>

#include "Tools/Exception/exception.hpp"
#include "Tools/Math/utils.h"

size_t
spu::tools::find_smallest_common_multiple(const size_t a, const size_t b)
{
    if (a == 0 || b == 0)
    {
        std::stringstream message;
        message << "'a' and 'b' have to be strictly positive ('a' = " << a << ", 'b' = " << b << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    size_t a2 = a, b2 = b;
    while (a2 != b2)
    {
        if (a2 > b2)
            b2 += b;
        else if (a2 < b2)
            a2 += a;
    }
    return a2;
}
