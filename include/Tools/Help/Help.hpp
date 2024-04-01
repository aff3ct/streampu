/*!
 * \file
 * \brief AFF3CT helper.
 */
#ifndef HELP_HPP_
#define HELP_HPP_

#include <iostream>
#include <sstream>

#include "Module/Module.hpp"
#include "Runtime/Socket/Socket.hpp"
#include "Runtime/Task/Task.hpp"

namespace aff3ct
{
namespace tools
{
/*! Print help for a module.
\param mdl A reference on the module.
\param verbose If true, more information is displayed.
\param stream Output stream to write help.
*/
void
help(const aff3ct::module::Module& mdl, const bool& verbose = false, std::ostream& stream = std::cout);

/*! Print help for a task.
\param tsk A reference on the task.
\param verbose If true, more information is displayed.
\param stream Output stream to write help.
*/
void
help(const aff3ct::runtime::Task& tsk, const bool& verbose = false, std::ostream& stream = std::cout);

/*! Print help for a socket.
\param sck A reference on the socket.
\param verbose If true, more information is displayed.
\param stream Output stream to write help.
*/
void
help(const aff3ct::runtime::Socket& sck, const bool& verbose = false, std::ostream& stream = std::cout);

/*! Get the help string for a module.
\param mdl A reference on the module.
\param verbose If true, more information is displayed.
\return The help string for the module object.
*/
std::string
get_help(const aff3ct::module::Module& mdl, const bool& verbose = false);

/*! Get the help string for a task.
\param tsk A reference on the task.
\param verbose If true, more information is displayed.
\return The help string for the task object.
*/
std::string
get_help(const aff3ct::runtime::Task& tsk, const bool& verbose = false);

/*! Get the help string for a socket.
\param sck A reference on the socket.
\param verbose If true, more information is displayed.
\return The help string for the socket object.
*/
std::string
get_help(const aff3ct::runtime::Socket& sck, const bool& verbose = false);
}
}

#endif /* HELP_HPP_ */
