/*!
 * \file
 * \brief Functions for overloading rang.
 */
#ifndef RANG_FORMAT_H_
#define RANG_FORMAT_H_

#include <iostream>
#include <rang.hpp>
#include <string>

namespace rang
{

enum class tag
{
    error = 0,
    warning,
    info,
    comment,
    positive
};

std::ostream&
operator<<(std::ostream& os, tag f);

void
format_on_each_line(std::ostream& os, const std::string& str, tag f);

}

#endif /* RANG_FORMAT_H_ */
