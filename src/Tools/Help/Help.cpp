#include "Tools/Help/Help.hpp"

using namespace aff3ct;
using namespace aff3ct::tools;

void aff3ct::tools::help(const aff3ct::module::Module& mdl, const bool& verbose, std::ostream& stream)
{
	stream << get_help(mdl, verbose) << std::endl;
}

void aff3ct::tools::help(const aff3ct::runtime::Task& tsk, const bool& verbose, std::ostream& stream)
{
	stream << get_help(tsk, verbose) << std::endl;
}

void aff3ct::tools::help(const aff3ct::runtime::Socket& sck, const bool& verbose, std::ostream& stream)
{
	stream << get_help(sck, verbose) << std::endl;
}

std::string aff3ct::tools::get_help(const aff3ct::module::Module& mdl, const bool& verbose)
{
	std::stringstream message;
	message << "# module:\n";
	message << "# |- name: " << mdl.get_name() << std::endl;
	message << "# |- short name: " << mdl.get_short_name() << std::endl;
	if (!mdl.get_custom_name().empty())
		message << "# |- custom name: " << mdl.get_custom_name() << std::endl;
	if (verbose)
		message << "# |- address: " << std::hex << static_cast<const void*>(&mdl) << std::endl;
	message << "# |- n_frames: " << mdl.get_n_frames() << "\n";
	if (mdl.tasks.size() > 0)
	{
		message << "# |- tasks:\n";

		for (size_t i = 0 ; i < mdl.tasks.size(); i++)
		{
			std::string tsk_msg = get_help(*mdl.tasks[i], verbose);
			std::string line;
			std::istringstream isstr(tsk_msg);
			while (std::getline(isstr, line))
				message << "# |  " << line.c_str() << "\n";
		}
	}
	return message.str();
}


std::string aff3ct::tools::get_help(const aff3ct::runtime::Task& tsk, const bool& verbose)
{
	std::stringstream message;

	message << "|- name: " << tsk.get_name() << std::endl;
	if (verbose)
		message << "|  |- address: " << std::hex << static_cast<const void*>(&tsk) << std::endl;

	if (tsk.sockets.size() > 0)
	{
		std::stringstream message_inputs;
		std::stringstream message_outputs;
		int n_inputs = 0;
		int n_outputs = 0;
		message_inputs << "|  |- sockets in:\n";
		message_outputs << "|  |- sockets out:\n";
		for (size_t i = 0 ; i < tsk.sockets.size() ; i++)
		{
			std::string sck_msg = get_help(*tsk.sockets[i], verbose);
			auto sck_type = tsk.sockets[i]->get_type();
			std::string line;
			std::istringstream isstr(sck_msg);
			if (sck_type == aff3ct::runtime::socket_t::SIN || sck_type == aff3ct::runtime::socket_t::SFWD)
			{
				n_inputs++;
				while (std::getline(isstr, line))
					message_inputs << "|  |  " << line.c_str() << "\n";
			}
			else
			{
				n_outputs++;
				while (std::getline(isstr, line))
					message_outputs << "|  |  " << line.c_str() << "\n";
			}
		}
		if (n_inputs > 0)
			message << message_inputs.str();
		if (n_outputs > 0)
			message << message_outputs.str();
	}
	return message.str();
}

std::string aff3ct::tools::get_help(const aff3ct::runtime::Socket& sck, const bool& verbose)
{
	std::stringstream message;
	message << "|- " << sck.get_name() << ": ";
	message << "{ n_elts = " << sck.get_n_elmts()/sck.get_task().get_module().get_n_frames();
	message << ", datatype = " << sck.get_datatype_string();
	if (verbose)
	{
		message << ", databytes = " << sck.get_databytes();
		message << ", dataptr = " << std::hex << sck.get_dataptr();
		message << ", address = " << std::hex << static_cast<const void*>(&sck);
	}
	message << " }\n";
	return message.str();
}