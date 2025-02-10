#include <sstream>
#include <string>

#include "Module/Stateful/Set/Set.hpp"
#include "Runtime/Sequence/Sequence.hpp"
#include "Tools/Exception/exception.hpp"

using namespace spu;
using namespace spu::module;

Set::Set(runtime::Sequence& sequence)
  : Stateful()
  , sequence_extern(&sequence)
{
    this->init();
}

Set::Set(const runtime::Sequence& sequence)
  : Stateful()
  , sequence_cloned(sequence.clone())
  , sequence_extern(nullptr)
{
    this->init();
}

void
Set::init()
{
    const std::string name = "Set";
    this->set_name(name);
    this->set_short_name(name);
    this->set_single_wave(true);

    auto& sequence = this->get_sequence();

    if (sequence.get_n_threads() != 1)
    {
        std::stringstream message;
        message << "'sequence.get_n_threads()' has to be equal to 1 ('sequence.get_n_threads()' = "
                << sequence.get_n_threads() << ").";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    auto& p = this->create_task("exec");
    p.set_autoalloc(true);

    auto& firsts = sequence.get_firsts_tasks()[0];
    for (auto& first : firsts)
        for (auto& s : first->sockets)
        {
            if (s->get_type() == runtime::socket_t::SIN)
            {
                if (s->get_datatype() == typeid(int8_t))
                    this->template create_socket_in<int8_t>(p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(uint8_t))
                    this->template create_socket_in<uint8_t>(p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(int16_t))
                    this->template create_socket_in<int16_t>(p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(uint16_t))
                    this->template create_socket_in<uint16_t>(
                      p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(int32_t))
                    this->template create_socket_in<int32_t>(p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(uint32_t))
                    this->template create_socket_in<uint32_t>(
                      p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(int64_t))
                    this->template create_socket_in<int64_t>(p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(uint64_t))
                    this->template create_socket_in<uint64_t>(
                      p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(float))
                    this->template create_socket_in<float>(p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(double))
                    this->template create_socket_in<double>(p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
            }
            else if (s->get_type() == runtime::socket_t::SFWD)
            {
                std::stringstream message;
                message << "Forward socket is not supported yet :-(.";
                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            }
        }
    auto& lasts = sequence.get_lasts_tasks()[0];
    for (auto& last : lasts)
        for (auto& s : last->sockets)
        {
            if (s->get_type() == runtime::socket_t::SOUT && s->get_name() != "status")
            {
                if (s->get_datatype() == typeid(int8_t))
                    this->template create_socket_out<int8_t>(p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(uint8_t))
                    this->template create_socket_out<uint8_t>(
                      p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(int16_t))
                    this->template create_socket_out<int16_t>(
                      p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(uint16_t))
                    this->template create_socket_out<uint16_t>(
                      p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(int32_t))
                    this->template create_socket_out<int32_t>(
                      p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(uint32_t))
                    this->template create_socket_out<uint32_t>(
                      p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(int64_t))
                    this->template create_socket_out<int64_t>(
                      p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(uint64_t))
                    this->template create_socket_out<uint64_t>(
                      p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(float))
                    this->template create_socket_out<float>(p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
                else if (s->get_datatype() == typeid(double))
                    this->template create_socket_out<double>(p, s->get_name(), s->get_n_elmts() / this->get_n_frames());
            }
            else if (s->get_type() == runtime::socket_t::SFWD)
            {
                std::stringstream message;
                message << "Forward socket is not supported yet :-(.";
                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            }
        }

    size_t sid = 0;
    for (auto& last : lasts)
        for (auto& s : last->sockets)
        {
            if (s->get_type() == runtime::socket_t::SOUT && s->get_name() != "status")
            {
                while (p.sockets[sid]->get_type() != runtime::socket_t::SOUT)
                    sid++;
                p.sockets[sid++]->_bind(*s); // out to out socket binding = black magic
            }
        }

    this->create_codelet(p,
                         [](Module& m, runtime::Task& t, const size_t /*frame_id*/) -> int
                         {
                             auto& ss = static_cast<Set&>(m);

                             auto& firsts = ss.get_sequence().get_firsts_tasks()[0];
                             size_t sid = 0;
                             for (auto& first : firsts)
                                 for (auto& s : first->sockets)
                                 {
                                     if (s->get_type() == runtime::socket_t::SIN)
                                     {
                                         while (t.sockets[sid]->get_type() != runtime::socket_t::SIN)
                                             sid++;
                                         (*s) = t.sockets[sid++]->_get_dataptr();
                                     }
                                 }

                             // execute all frames sequentially
                             ss.get_sequence().exec_seq();

                             return runtime::status_t::SUCCESS;
                         });
}

runtime::Sequence&
Set::get_sequence()
{
    if (this->sequence_extern)
        return *this->sequence_extern;
    else
        return *this->sequence_cloned;
}

Set*
Set::clone() const
{
    auto m = new Set(*this);
    m->deep_copy(*this);
    return m;
}

void
Set::deep_copy(const Set& m)
{
    Stateful::deep_copy(m);
    if (m.sequence_cloned != nullptr)
        this->sequence_cloned.reset(m.sequence_cloned->clone());
    else
    {
        this->sequence_cloned.reset(m.sequence_extern->clone());
        this->sequence_extern = nullptr;
    }

    auto& lasts = this->get_sequence().get_lasts_tasks()[0];

    try
    {
        auto& p = (*this)("exec");

        size_t sid = 0;
        for (auto& last : lasts)
            for (auto& s : last->sockets)
            {
                if (s->get_type() == runtime::socket_t::SOUT && s->get_name() != "status")
                {
                    while (p.sockets[sid]->get_type() != runtime::socket_t::SOUT)
                        sid++;
                    p.sockets[sid++]->_bind(*s); // out to out socket binding = black magic
                }
            }
    }
    catch (tools::invalid_argument&)
    {
        /* this is a hack: do nothing, we went there because of trying to determine if the set is replicable */
    }
}

void
Set::set_n_frames(const size_t n_frames)
{
    const auto old_n_frames = this->get_n_frames();
    if (old_n_frames != n_frames)
    {
        auto& p = *this->tasks[0];
        auto& lasts = this->get_sequence().get_lasts_tasks()[0];
        size_t sid = 0;
        for (auto& last : lasts)
            for (auto& s : last->sockets)
            {
                if (s->get_type() == runtime::socket_t::SOUT && s->get_name() != "status")
                {
                    while (p.sockets[sid]->get_type() != runtime::socket_t::SOUT)
                        sid++;
                    p.sockets[sid++]->unbind(*s);
                }
            }

        Module::set_n_frames(n_frames);

        if (this->sequence_extern)
            this->sequence_extern->set_n_frames(n_frames);
        else
            this->sequence_cloned->set_n_frames(n_frames);

        sid = 0;
        for (auto& last : lasts)
            for (auto& s : last->sockets)
            {
                if (s->get_type() == runtime::socket_t::SOUT && s->get_name() != "status")
                {
                    while (p.sockets[sid]->get_type() != runtime::socket_t::SOUT)
                        sid++;
                    p.sockets[sid++]->_bind(*s); // out to out socket binding = black magic
                }
            }
    }
}
