#include "Help.hpp"

namespace aff3ct
{
namespace tools
{
    void help(const aff3ct::module::Module& mdl, const bool& verbose, std::ostream& stream)
    {
        stream << get_help(mdl, verbose) << std::endl;
    }

    void help(const aff3ct::runtime::Task& tsk, const bool& verbose, std::ostream& stream)
    {
        stream << get_help(tsk, verbose) << std::endl;
    }

    void help(const aff3ct::runtime::Socket& sck, const bool& verbose, std::ostream& stream)
    {
        stream << get_help(sck, verbose) << std::endl;
    }

    std::string get_help(const aff3ct::module::Module& mdl, const bool& verbose)
    {
        std::stringstream message;
        message << mdl.get_name();
        if (verbose)
            message << ", at address " << std::hex << static_cast<const void*>(&mdl);

        message << "\n";
        message << "|  Attributes\n";
        message << "|  ----------\n";
        message << "|  n_frames: "    << mdl.get_n_frames() << "\n|\n";
        if (mdl.tasks.size() > 0)
        {
            message << "|  Tasks\n";
            message << "|  -----\n";

            for (size_t i = 0 ; i < mdl.tasks.size() ; i++)
            {
                std::string tsk_msg = get_help(*mdl.tasks[i], verbose);
                std::string line;
                std::istringstream isstr(tsk_msg);
                while (std::getline(isstr, line))
                    message << "|  " << line.c_str() << "\n";
                message << "|\n";
            }
        }
        return message.str();
    }


    std::string get_help(const aff3ct::runtime::Task& tsk, const bool& verbose)
    {
        std::stringstream message;

        message << tsk.get_name();
        if (verbose)
            message << ", at address " << std::hex << static_cast<const void*>(&tsk);

        message << "\n";
        if (tsk.sockets.size() > 0)
        {
            std::stringstream message_inputs;
            std::stringstream message_outputs;
            int n_inputs = 0;
            int n_outputs = 0;
            message_inputs << "  Input sockets\n";
            message_inputs << "  -------------\n";
            message_outputs << "  Output sockets\n";
            message_outputs << "  --------------\n";
            for (size_t i = 0 ; i < tsk.sockets.size() ; i++)
            {
                std::string sck_msg = get_help(*tsk.sockets[i], verbose);
                auto sck_type = tsk.get_socket_type(*tsk.sockets[i]);
                std::string line;
                std::istringstream isstr(sck_msg);
                if (sck_type == aff3ct::runtime::socket_t::SIN)
                {
                    n_inputs++;
                    while (std::getline(isstr, line))
                        message_inputs << "  " << line.c_str() << "\n";

                }
                else
                {
                    n_outputs++;
                    while (std::getline(isstr, line))
                        message_outputs << "  " << line.c_str() << "\n";
                }
            }
            if (n_inputs > 0)
                message << message_inputs.str() << "\n";
            if (n_outputs > 0)
                message << message_outputs.str();
        }
        return message.str();
    }

    std::string get_help(const aff3ct::runtime::Socket& sck, const bool& verbose)
    {
        std::stringstream message;
        message << sck.get_name() << " : ";
        message << "Socket(n_elts = " << sck.get_n_elmts()/sck.get_task().get_module().get_n_frames();
        message << ", datatype = " << sck.get_datatype_string();
        if (verbose){
            message << ", databytes = " << sck.get_databytes();
            message << ", dataptr = " << std::hex<< sck.get_dataptr();
            message << ", address = " << std::hex << static_cast<const void*>(&sck);
        }
        message << ")\n";
        return message.str();
    }
}
}