#include <algorithm>
#include <iomanip>
#include <ios>
#include <iostream>
#include <rang.hpp>
#include <sstream>
#include <typeinfo>

#include "Module/Module.hpp"
#include "Runtime/Socket/Socket.hpp"
#include "Runtime/Task/Task.hpp"
#include "Tools/Exception/exception.hpp"

using namespace spu;
using namespace spu::runtime;

Task::Task(module::Module& module,
           const std::string& name,
           const bool stats,
           const bool fast,
           const bool debug,
           const bool outbuffers_allocated)
  : module(&module)
  , name(name)
  , stats(stats)
  , fast(fast)
  , debug(debug)
  , outbuffers_allocated(outbuffers_allocated)
  , debug_hex(false)
  , replicable(module.is_clonable())
  , debug_limit(-1)
  , debug_precision(2)
  , debug_frame_max(-1)
  , codelet(
      [](module::Module& /*m*/, Task& /*t*/, const size_t /*frame_id*/) -> int
      {
          throw tools::unimplemented_error(__FILE__, __LINE__, __func__);
          return 0;
      })
  , n_input_sockets(0)
  , n_output_sockets(0)
  , n_fwd_sockets(0)
  , status(module.get_n_waves())
  , n_calls(0)
  , duration_total(std::chrono::nanoseconds(0))
  , duration_min(std::chrono::nanoseconds(0))
  , duration_max(std::chrono::nanoseconds(0))
  , last_input_socket(nullptr)
{
}

Socket&
Task::operator[](const std::string& sck_name)
{
    std::string s_name = sck_name;
    s_name.erase(remove(s_name.begin(), s_name.end(), ' '), s_name.end());

    auto it = find_if(this->sockets.begin(),
                      this->sockets.end(),
                      [s_name](std::shared_ptr<runtime::Socket> s) { return s->get_name() == s_name; });

    if (it == this->sockets.end())
    {
        std::stringstream message;
        message << "runtime::Socket '" << s_name << "' not found for task '" << this->get_name() << "'.";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    return *it->get();
}

void
Task::set_stats(const bool stats)
{
    this->stats = stats;
}

void
Task::set_fast(const bool fast)
{
    this->fast = fast;
    for (size_t i = 0; i < sockets.size(); i++)
        sockets[i]->set_fast(this->fast);
}

void
Task::set_debug(const bool debug)
{
    this->debug = debug;
}

void
Task::set_debug_hex(const bool debug_hex)
{
    this->debug_hex = debug_hex;
}

void
Task::set_debug_limit(const uint32_t limit)
{
    this->debug_limit = (int32_t)limit;
}

void
Task::set_debug_precision(const uint8_t prec)
{
    this->debug_precision = prec;
}

void
Task::set_debug_frame_max(const uint32_t limit)
{
    this->debug_frame_max = limit;
}

// trick to compile on the GNU compiler version 4 (where 'std::hexfloat' is unavailable)
#if !defined(__clang__) && !defined(__llvm__) && defined(__GNUC__) && defined(__cplusplus) && __GNUC__ < 5
namespace std
{
class Hexfloat
{
  public:
    void message(std::ostream& os) const { os << " /!\\ 'std::hexfloat' is not supported by this compiler. /!\\ "; }
};
Hexfloat hexfloat;
}
std::ostream&
operator<<(std::ostream& os, const std::Hexfloat& obj)
{
    obj.message(os);
    return os;
}
#endif

template<typename T>
static inline void
display_data(const T* data,
             const size_t fra_size,
             const size_t n_fra,
             const size_t n_fra_per_w,
             const size_t limit,
             const size_t max_frame,
             const uint8_t p,
             const uint8_t n_spaces,
             const bool hex)
{
    constexpr bool is_float_type = std::is_same<float, T>::value || std::is_same<double, T>::value;

    std::ios::fmtflags f(std::cout.flags());
    if (hex)
    {
        if (is_float_type)
            std::cout << std::hexfloat << std::hex;
        else
            std::cout << std::hex;
    }
    else
        std::cout << std::fixed << std::setprecision(p) << std::dec;

    if (n_fra == 1 && max_frame != 0)
    {
        for (size_t i = 0; i < limit; i++)
        {
            if (hex)
                std::cout << (!is_float_type ? "0x" : "") << +data[i] << (i < limit - 1 ? ", " : "");
            else
                std::cout << std::setw(p + 3) << +data[i] << (i < limit - 1 ? ", " : "");
        }
        std::cout << (limit < fra_size ? ", ..." : "");
    }
    else
    {
        std::string spaces = "#";
        for (uint8_t s = 0; s < n_spaces - 1; s++)
            spaces += " ";

        auto n_digits_dec = [](size_t f) -> size_t
        {
            size_t count = 0;
            while (f && ++count)
                f /= 10;
            return count;
        };

        const auto n_digits = n_digits_dec(max_frame);
        auto ftos = [&n_digits_dec, &n_digits](size_t f) -> std::string
        {
            auto n_zero = n_digits - n_digits_dec(f);
            std::string f_str = "";
            for (size_t z = 0; z < n_zero; z++)
                f_str += "0";
            f_str += std::to_string(f);
            return f_str;
        };

        const auto n_digits_w = n_digits_dec((max_frame / n_fra_per_w) == 0 ? 1 : (max_frame / n_fra_per_w));
        auto wtos = [&n_digits_dec, &n_digits_w](size_t w) -> std::string
        {
            auto n_zero = n_digits_w - n_digits_dec(w);
            std::string f_str = "";
            for (size_t z = 0; z < n_zero; z++)
                f_str += "0";
            f_str += std::to_string(w);
            return f_str;
        };

        for (size_t f = 0; f < max_frame; f++)
        {
            const auto w = f / n_fra_per_w;
            std::cout << (f >= 1 ? spaces : "") << rang::style::bold << rang::fg::gray << "f" << ftos(f + 1) << "_w"
                      << wtos(w + 1) << rang::style::reset << "(";

            for (size_t i = 0; i < limit; i++)
            {
                if (hex)
                    std::cout << (!is_float_type ? "0x" : "") << +data[f * fra_size + i] << (i < limit - 1 ? ", " : "");
                else
                    std::cout << std::setw(p + 3) << +data[f * fra_size + i] << (i < limit - 1 ? ", " : "");
            }
            std::cout << (limit < fra_size ? ", ..." : "") << ")" << (f < n_fra - 1 ? ", \n" : "");
        }

        if (max_frame < n_fra)
        {
            const auto w1 = max_frame / n_fra_per_w;
            const auto w2 = n_fra / n_fra_per_w;
            std::cout << (max_frame >= 1 ? spaces : "") << rang::style::bold << rang::fg::gray << "f"
                      << std::setw(n_digits) << max_frame + 1 << "_w" << std::setw(n_digits_w) << w1 + 1 << "->"
                      << "f" << std::setw(n_digits) << n_fra << "_w" << std::setw(n_digits_w) << w2 + 1 << ":"
                      << rang::style::reset << "(...)";
        }
    }

    std::cout.flags(f);
}

void
Task::_exec(const int frame_id, const bool managed_memory)
{
    const auto n_frames = this->get_module().get_n_frames();
    const auto n_frames_per_wave = this->get_module().get_n_frames_per_wave();
    const auto n_waves = this->get_module().get_n_waves();
    const auto n_frames_per_wave_rest = this->get_module().get_n_frames_per_wave_rest();

    // do not use 'this->status' because the dataptr can have been changed by the 'tools::Sequence' when using the no
    // copy mode
    int* status = this->sockets.back()->get_dataptr<int>();
    for (size_t w = 0; w < n_waves; w++)
        status[w] = (int)status_t::UNKNOWN;

    if ((managed_memory == false && frame_id >= 0) || (frame_id == -1 && n_frames_per_wave == n_frames) ||
        (frame_id == 0 && n_frames_per_wave == 1) || (frame_id == 0 && n_waves > 1) ||
        (frame_id == 0 && n_frames_per_wave_rest == 0))
    {
        const auto real_frame_id = frame_id == -1 ? 0 : frame_id;
        const size_t w = (real_frame_id % n_frames) / n_frames_per_wave;
        status[w] = this->codelet(*this->module, *this, real_frame_id);
    }
    else
    {
        // save the initial dataptr of the sockets
        for (size_t sid = 0; sid < this->sockets.size() - 1; sid++)
            sockets_dataptr_init[sid] = (int8_t*)this->sockets[sid]->_get_dataptr();

        if (frame_id > 0 && managed_memory == true && n_frames_per_wave > 1)
        {
            const size_t w = (frame_id % n_frames) / n_frames_per_wave;
            const size_t w_pos = frame_id % n_frames_per_wave;

            for (size_t sid = 0; sid < this->sockets.size() - 1; sid++)
            {
                if (sockets[sid]->get_type() == socket_t::SIN || sockets[sid]->get_type() == socket_t::SFWD)
                    std::copy(
                      sockets_dataptr_init[sid] + ((frame_id % n_frames) + 0) * sockets_databytes_per_frame[sid],
                      sockets_dataptr_init[sid] + ((frame_id % n_frames) + 1) * sockets_databytes_per_frame[sid],
                      sockets_data[sid].begin() + w_pos * sockets_databytes_per_frame[sid]);
                this->sockets[sid]->dataptr = (void*)sockets_data[sid].data();
            }

            status[w] = this->codelet(*this->module, *this, w * n_frames_per_wave);

            for (size_t sid = 0; sid < this->sockets.size() - 1; sid++)
                if (sockets[sid]->get_type() == socket_t::SOUT || sockets[sid]->get_type() == socket_t::SFWD)
                    std::copy(sockets_data[sid].begin() + (w_pos + 0) * sockets_databytes_per_frame[sid],
                              sockets_data[sid].begin() + (w_pos + 1) * sockets_databytes_per_frame[sid],
                              sockets_dataptr_init[sid] + (frame_id % n_frames) * sockets_databytes_per_frame[sid]);
        }
        else // if (frame_id <= 0 || n_frames_per_wave == 1)
        {
            const size_t w_start = (frame_id < 0) ? 0 : frame_id % n_waves;
            const size_t w_stop = (frame_id < 0) ? n_waves : w_start + 1;

            size_t w = 0;
            auto exec_status = status_t::SUCCESS;
            for (w = w_start; w < w_stop - 1 && exec_status != status_t::FAILURE_STOP; w++)
            {
                for (size_t sid = 0; sid < this->sockets.size() - 1; sid++)
                    this->sockets[sid]->dataptr =
                      (void*)(sockets_dataptr_init[sid] + w * n_frames_per_wave * sockets_databytes_per_frame[sid]);

                status[w] = this->codelet(*this->module, *this, w * n_frames_per_wave);
                exec_status = (status_t)status[w];
            }

            if (exec_status != status_t::FAILURE_STOP)
            {
                if (n_frames_per_wave_rest == 0)
                {
                    for (size_t sid = 0; sid < this->sockets.size() - 1; sid++)
                        this->sockets[sid]->dataptr =
                          (void*)(sockets_dataptr_init[sid] + w * n_frames_per_wave * sockets_databytes_per_frame[sid]);

                    status[w] = this->codelet(*this->module, *this, w * n_frames_per_wave);
                }
                else
                {
                    for (size_t sid = 0; sid < this->sockets.size() - 1; sid++)
                    {
                        if (sockets[sid]->get_type() == socket_t::SIN || sockets[sid]->get_type() == socket_t::SFWD)
                            std::copy(sockets_dataptr_init[sid] +
                                        w * n_frames_per_wave * sockets_databytes_per_frame[sid],
                                      sockets_dataptr_init[sid] + n_frames * sockets_databytes_per_frame[sid],
                                      sockets_data[sid].begin());
                        this->sockets[sid]->dataptr = (void*)sockets_data[sid].data();
                    }

                    status[w] = this->codelet(*this->module, *this, w * n_frames_per_wave);

                    for (size_t sid = 0; sid < this->sockets.size() - 1; sid++)
                        if (sockets[sid]->get_type() == socket_t::SOUT || sockets[sid]->get_type() == socket_t::SFWD)
                            std::copy(
                              sockets_data[sid].begin(),
                              sockets_data[sid].begin() + n_frames_per_wave_rest * sockets_databytes_per_frame[sid],
                              sockets_dataptr_init[sid] + w * n_frames_per_wave * sockets_databytes_per_frame[sid]);
                }
            }
        }

        // restore the initial dataptr of the sockets
        for (size_t sid = 0; sid < this->sockets.size() - 1; sid++)
            this->sockets[sid]->dataptr = (void*)sockets_dataptr_init[sid];
    }
}

const std::vector<int>&
Task::exec(const int frame_id, const bool managed_memory)
{
#ifndef SPU_FAST
    if (this->is_fast() && !this->is_debug() && !this->is_stats())
    {
#endif
        this->_exec(frame_id, managed_memory);
        this->n_calls++;
        return this->get_status();
#ifndef SPU_FAST
    }

    if (frame_id != -1 && (size_t)frame_id >= this->get_module().get_n_frames())
    {
        std::stringstream message;
        message << "'frame_id' has to be equal to '-1' or to be smaller than 'n_frames' ('frame_id' = " << frame_id
                << ", 'n_frames' = " << this->get_module().get_n_frames() << ").";
        throw tools::length_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (this->is_fast() || this->can_exec())
    {
        size_t max_n_chars = 0;
        if (this->is_debug())
        {
            auto n_fra = this->module->get_n_frames();
            auto n_fra_per_w = this->module->get_n_frames_per_wave();

            std::string module_name =
              module->get_custom_name().empty() ? module->get_name() : module->get_custom_name();

            std::cout << "# ";
            std::cout << rang::style::bold << rang::fg::green << module_name << rang::style::reset
                      << "::" << rang::style::bold << rang::fg::magenta << get_name() << rang::style::reset << "(";
            for (auto i = 0; i < (int)sockets.size() - 1; i++)
            {
                auto& s = *sockets[i];
                auto s_type = s.get_type();
                auto n_elmts = s.get_databytes() / (size_t)s.get_datatype_size();
                std::cout << rang::style::bold << rang::fg::blue << (s_type == socket_t::SIN ? "const " : "")
                          << s.get_datatype_string() << rang::style::reset << " " << s.get_name() << "["
                          << (n_fra > 1 ? std::to_string(n_fra) + "x" : "") << (n_elmts / n_fra) << "]"
                          << (i < (int)sockets.size() - 2 ? ", " : "");

                max_n_chars = std::max(s.get_name().size(), max_n_chars);
            }
            std::cout << ")" << std::endl;

            for (auto& s : sockets)
            {
                auto s_type = s->get_type();
                if (s_type == socket_t::SIN || s_type == socket_t::SFWD)
                {
                    std::string spaces;
                    for (size_t ss = 0; ss < max_n_chars - s->get_name().size(); ss++)
                        spaces += " ";

                    auto n_elmts = s->get_databytes() / (size_t)s->get_datatype_size();
                    auto fra_size = n_elmts / n_fra;
                    auto limit = debug_limit != -1 ? std::min(fra_size, (size_t)debug_limit) : fra_size;
                    auto max_frame = debug_frame_max != -1 ? std::min(n_fra, (size_t)debug_frame_max) : n_fra;
                    auto p = debug_precision;
                    auto h = debug_hex;
                    std::cout << "# {IN}  " << s->get_name() << spaces << " = [";
                    if (s->get_datatype() == typeid(int8_t))
                        display_data(s->get_dataptr<const int8_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(uint8_t))
                        display_data(s->get_dataptr<const uint8_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(int16_t))
                        display_data(s->get_dataptr<const int16_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(uint16_t))
                        display_data(s->get_dataptr<const uint16_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(int32_t))
                        display_data(s->get_dataptr<const int32_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(uint32_t))
                        display_data(s->get_dataptr<const uint32_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(int64_t))
                        display_data(s->get_dataptr<const int64_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(uint64_t))
                        display_data(s->get_dataptr<const uint64_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(float))
                        display_data(s->get_dataptr<const float>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(double))
                        display_data(s->get_dataptr<const double>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    std::cout << "]" << std::endl;
                }
            }
        }

        if (this->is_stats())
        {
            auto t_start = std::chrono::steady_clock::now();
            this->_exec(frame_id, managed_memory);
            auto duration = std::chrono::steady_clock::now() - t_start;

            this->duration_total += duration;
            if (n_calls)
            {
                this->duration_min = std::min(this->duration_min, duration);
                this->duration_max = std::max(this->duration_max, duration);
            }
            else
            {
                this->duration_min = duration;
                this->duration_max = duration;
            }
        }
        else
        {
            this->_exec(frame_id, managed_memory);
        }
        this->n_calls++;

        if (this->is_debug())
        {
            auto n_fra = this->module->get_n_frames();
            auto n_fra_per_w = this->module->get_n_frames_per_wave();
            for (auto& s : sockets)
            {
                auto s_type = s->get_type();
                if ((s_type == socket_t::SOUT) && s->get_name() != "status")
                {
                    std::string spaces;
                    for (size_t ss = 0; ss < max_n_chars - s->get_name().size(); ss++)
                        spaces += " ";

                    auto n_elmts = s->get_databytes() / (size_t)s->get_datatype_size();
                    auto fra_size = n_elmts / n_fra;
                    auto limit = debug_limit != -1 ? std::min(fra_size, (size_t)debug_limit) : fra_size;
                    auto max_frame = debug_frame_max != -1 ? std::min(n_fra, (size_t)debug_frame_max) : n_fra;
                    auto p = debug_precision;
                    auto h = debug_hex;
                    std::cout << "# {OUT} " << s->get_name() << spaces << " = [";
                    if (s->get_datatype() == typeid(int8_t))
                        display_data(s->get_dataptr<const int8_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(uint8_t))
                        display_data(s->get_dataptr<const uint8_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(int16_t))
                        display_data(s->get_dataptr<const int16_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(uint16_t))
                        display_data(s->get_dataptr<const uint16_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(int32_t))
                        display_data(s->get_dataptr<const int32_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(uint32_t))
                        display_data(s->get_dataptr<const uint32_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(int64_t))
                        display_data(s->get_dataptr<const int64_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(uint64_t))
                        display_data(s->get_dataptr<const uint64_t>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(float))
                        display_data(s->get_dataptr<const float>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    else if (s->get_datatype() == typeid(double))
                        display_data(s->get_dataptr<const double>(),
                                     fra_size,
                                     n_fra,
                                     n_fra_per_w,
                                     limit,
                                     max_frame,
                                     p,
                                     (uint8_t)max_n_chars + 12,
                                     h);
                    std::cout << "]" << std::endl;
                }
            }
            std::cout << "# Returned status: [";
            // do not use 'this->status' because the dataptr can have been changed by the 'tools::Sequence' when using
            // the no copy mode
            int* status = this->sockets.back()->get_dataptr<int>();
            for (size_t w = 0; w < this->get_module().get_n_waves(); w++)
            {
                if (status_t_to_string.count(status[w]))
                    std::cout << ((w != 0) ? ", " : "") << std::dec << status[w] << " '"
                              << status_t_to_string[status[w]] << "'";
                else
                    std::cout << ((w != 0) ? ", " : "") << std::dec << status[w];
            }
            std::cout << "]" << std::endl;
            std::cout << "#" << std::noshowbase << std::endl;
        }

        // if (exec_status < 0)
        // {
        //  std::stringstream message;
        //  message << "'exec_status' can't be negative ('exec_status' = " << exec_status << ").";
        //  throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        // }

        return this->get_status();
    }
    else
    {
        std::stringstream socs;
        socs << "'socket(s).name' = [";
        auto s = 0;
        for (size_t i = 0; i < sockets.size(); i++)
            if (sockets[i]->dataptr == nullptr) socs << (s != 0 ? ", " : "") << sockets[i]->name;
        socs << "]";

        std::stringstream message;
        message << "The task cannot be executed because some of the inputs/output sockets are not fed ('task.name' = "
                << this->get_name() << ", 'module.name' = " << module->get_name() << ", " << socs.str() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif /* !SPU_FAST */
}

template<typename T>
Socket&
Task::create_2d_socket(const std::string& name,
                       const size_t n_rows,
                       const size_t n_cols,
                       const socket_t type,
                       const bool hack_status)
{
    if (name.empty())
    {
        std::stringstream message;
        message << "Impossible to create this socket because the name is empty ('task.name' = " << this->get_name()
                << ", 'module.name' = " << module->get_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (name == "status" && !hack_status)
    {
        std::stringstream message;
        message << "A socket can't be named 'status'.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    for (auto& s : sockets)
        if (s->get_name() == name)
        {
            std::stringstream message;
            message << "Impossible to create this socket because an other socket has the same name ('socket.name' = "
                    << name << ", 'task.name' = " << this->get_name() << ", 'module.name' = " << module->get_name()
                    << ").";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

    for (auto s : this->sockets)
        if (s->get_name() == "status")
        {
            std::stringstream message;
            message << "Creating new sockets after the 'status' socket is forbidden.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }

    std::pair<size_t, size_t> databytes_per_dim = { n_rows, n_cols * sizeof(T) };
    auto s = std::make_shared<Socket>(*this, name, typeid(T), databytes_per_dim, type, this->is_fast());

    sockets.push_back(std::move(s));

    this->sockets_dataptr_init.push_back(nullptr);
    this->sockets_databytes_per_frame.push_back(sockets.back()->get_databytes() / this->get_module().get_n_frames());
    this->sockets_data.push_back(
      std::vector<int8_t>((this->get_module().get_n_frames_per_wave() > 1)
                            ? this->sockets_databytes_per_frame.back() * this->get_module().get_n_frames_per_wave()
                            : 0));

    return *sockets.back();
}

template<typename T>
size_t
Task::create_2d_socket_in(const std::string& name, const size_t n_rows, const size_t n_cols)
{
    auto& s = create_2d_socket<T>(name, n_rows, n_cols, socket_t::SIN);
    last_input_socket = &s;

    this->n_input_sockets++;

    return sockets.size() - 1;
}

size_t
Task::create_2d_socket_in(const std::string& name,
                          const size_t n_rows,
                          const size_t n_cols,
                          const std::type_index& datatype)
{
    if (datatype == typeid(int8_t))
        return this->template create_2d_socket_in<int8_t>(name, n_rows, n_cols);
    else if (datatype == typeid(uint8_t))
        return this->template create_2d_socket_in<uint8_t>(name, n_rows, n_cols);
    else if (datatype == typeid(int16_t))
        return this->template create_2d_socket_in<int16_t>(name, n_rows, n_cols);
    else if (datatype == typeid(uint16_t))
        return this->template create_2d_socket_in<uint16_t>(name, n_rows, n_cols);
    else if (datatype == typeid(int32_t))
        return this->template create_2d_socket_in<int32_t>(name, n_rows, n_cols);
    else if (datatype == typeid(uint32_t))
        return this->template create_2d_socket_in<uint32_t>(name, n_rows, n_cols);
    else if (datatype == typeid(int64_t))
        return this->template create_2d_socket_in<int64_t>(name, n_rows, n_cols);
    else if (datatype == typeid(uint64_t))
        return this->template create_2d_socket_in<uint64_t>(name, n_rows, n_cols);
    else if (datatype == typeid(float))
        return this->template create_2d_socket_in<float>(name, n_rows, n_cols);
    else if (datatype == typeid(double))
        return this->template create_2d_socket_in<double>(name, n_rows, n_cols);
    else
    {
        std::stringstream message;
        message << "This should never happen.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
}

size_t
Task::create_2d_socket_in(const std::string& name, const size_t n_rows, const size_t n_cols, const datatype_t datatype)
{
    switch (datatype)
    {
        case datatype_t::F64:
            return this->template create_2d_socket_in<double>(name, n_rows, n_cols);
            break;
        case datatype_t::F32:
            return this->template create_2d_socket_in<float>(name, n_rows, n_cols);
            break;
        case datatype_t::S64:
            return this->template create_2d_socket_in<int64_t>(name, n_rows, n_cols);
            break;
        case datatype_t::S32:
            return this->template create_2d_socket_in<int32_t>(name, n_rows, n_cols);
            break;
        case datatype_t::S16:
            return this->template create_2d_socket_in<int16_t>(name, n_rows, n_cols);
            break;
        case datatype_t::S8:
            return this->template create_2d_socket_in<int8_t>(name, n_rows, n_cols);
            break;
        case datatype_t::U64:
            return this->template create_2d_socket_in<uint64_t>(name, n_rows, n_cols);
            break;
        case datatype_t::U32:
            return this->template create_2d_socket_in<uint32_t>(name, n_rows, n_cols);
            break;
        case datatype_t::U16:
            return this->template create_2d_socket_in<uint16_t>(name, n_rows, n_cols);
            break;
        case datatype_t::U8:
            return this->template create_2d_socket_in<uint8_t>(name, n_rows, n_cols);
            break;
        default:
        {
            std::stringstream message;
            message << "This should never happen.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            break;
        }
    }
}

template<typename T>
size_t
Task::create_2d_socket_out(const std::string& name, const size_t n_rows, const size_t n_cols, const bool hack_status)
{
    create_2d_socket<T>(name, n_rows, n_cols, socket_t::SOUT, hack_status);
    this->n_output_sockets++;

    return sockets.size() - 1;
}

size_t
Task::create_2d_socket_out(const std::string& name,
                           const size_t n_rows,
                           const size_t n_cols,
                           const std::type_index& datatype,
                           const bool hack_status)
{
    if (datatype == typeid(int8_t))
        return this->template create_2d_socket_out<int8_t>(name, n_rows, n_cols, hack_status);
    else if (datatype == typeid(uint8_t))
        return this->template create_2d_socket_out<uint8_t>(name, n_rows, n_cols, hack_status);
    else if (datatype == typeid(int16_t))
        return this->template create_2d_socket_out<int16_t>(name, n_rows, n_cols, hack_status);
    else if (datatype == typeid(uint16_t))
        return this->template create_2d_socket_out<uint16_t>(name, n_rows, n_cols, hack_status);
    else if (datatype == typeid(int32_t))
        return this->template create_2d_socket_out<int32_t>(name, n_rows, n_cols, hack_status);
    else if (datatype == typeid(uint32_t))
        return this->template create_2d_socket_out<uint32_t>(name, n_rows, n_cols, hack_status);
    else if (datatype == typeid(int64_t))
        return this->template create_2d_socket_out<int64_t>(name, n_rows, n_cols, hack_status);
    else if (datatype == typeid(uint64_t))
        return this->template create_2d_socket_out<uint64_t>(name, n_rows, n_cols, hack_status);
    else if (datatype == typeid(float))
        return this->template create_2d_socket_out<float>(name, n_rows, n_cols, hack_status);
    else if (datatype == typeid(double))
        return this->template create_2d_socket_out<double>(name, n_rows, n_cols, hack_status);
    else
    {
        std::stringstream message;
        message << "This should never happen.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
}

size_t
Task::create_2d_socket_out(const std::string& name,
                           const size_t n_rows,
                           const size_t n_cols,
                           const datatype_t datatype,
                           const bool hack_status)
{
    switch (datatype)
    {
        case datatype_t::F64:
            return this->template create_2d_socket_out<double>(name, n_rows, n_cols, hack_status);
            break;
        case datatype_t::F32:
            return this->template create_2d_socket_out<float>(name, n_rows, n_cols, hack_status);
            break;
        case datatype_t::S64:
            return this->template create_2d_socket_out<int64_t>(name, n_rows, n_cols, hack_status);
            break;
        case datatype_t::S32:
            return this->template create_2d_socket_out<int32_t>(name, n_rows, n_cols, hack_status);
            break;
        case datatype_t::S16:
            return this->template create_2d_socket_out<int16_t>(name, n_rows, n_cols, hack_status);
            break;
        case datatype_t::S8:
            return this->template create_2d_socket_out<int8_t>(name, n_rows, n_cols, hack_status);
            break;
        case datatype_t::U64:
            return this->template create_2d_socket_out<uint64_t>(name, n_rows, n_cols, hack_status);
            break;
        case datatype_t::U32:
            return this->template create_2d_socket_out<uint32_t>(name, n_rows, n_cols, hack_status);
            break;
        case datatype_t::U16:
            return this->template create_2d_socket_out<uint16_t>(name, n_rows, n_cols, hack_status);
            break;
        case datatype_t::U8:
            return this->template create_2d_socket_out<uint8_t>(name, n_rows, n_cols, hack_status);
            break;
        default:
        {
            std::stringstream message;
            message << "This should never happen.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            break;
        }
    }
}

template<typename T>
size_t
Task::create_2d_socket_fwd(const std::string& name, const size_t n_rows, const size_t n_cols)
{
    auto& s = create_2d_socket<T>(name, n_rows, n_cols, socket_t::SFWD);
    last_input_socket = &s;

    this->n_fwd_sockets++;

    return sockets.size() - 1;
}

size_t
Task::create_2d_socket_fwd(const std::string& name,
                           const size_t n_rows,
                           const size_t n_cols,
                           const std::type_index& datatype)
{
    if (datatype == typeid(int8_t))
        return this->template create_2d_socket_fwd<int8_t>(name, n_rows, n_cols);
    else if (datatype == typeid(uint8_t))
        return this->template create_2d_socket_fwd<uint8_t>(name, n_rows, n_cols);
    else if (datatype == typeid(int16_t))
        return this->template create_2d_socket_fwd<int16_t>(name, n_rows, n_cols);
    else if (datatype == typeid(uint16_t))
        return this->template create_2d_socket_fwd<uint16_t>(name, n_rows, n_cols);
    else if (datatype == typeid(int32_t))
        return this->template create_2d_socket_fwd<int32_t>(name, n_rows, n_cols);
    else if (datatype == typeid(uint32_t))
        return this->template create_2d_socket_fwd<uint32_t>(name, n_rows, n_cols);
    else if (datatype == typeid(int64_t))
        return this->template create_2d_socket_fwd<int64_t>(name, n_rows, n_cols);
    else if (datatype == typeid(uint64_t))
        return this->template create_2d_socket_fwd<uint64_t>(name, n_rows, n_cols);
    else if (datatype == typeid(float))
        return this->template create_2d_socket_fwd<float>(name, n_rows, n_cols);
    else if (datatype == typeid(double))
        return this->template create_2d_socket_fwd<double>(name, n_rows, n_cols);
    else
    {
        std::stringstream message;
        message << "This should never happen.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
}

size_t
Task::create_2d_socket_fwd(const std::string& name, const size_t n_rows, const size_t n_cols, const datatype_t datatype)
{
    switch (datatype)
    {
        case datatype_t::F64:
            return this->template create_2d_socket_fwd<double>(name, n_rows, n_cols);
            break;
        case datatype_t::F32:
            return this->template create_2d_socket_fwd<float>(name, n_rows, n_cols);
            break;
        case datatype_t::S64:
            return this->template create_2d_socket_fwd<int64_t>(name, n_rows, n_cols);
            break;
        case datatype_t::S32:
            return this->template create_2d_socket_fwd<int32_t>(name, n_rows, n_cols);
            break;
        case datatype_t::S16:
            return this->template create_2d_socket_fwd<int16_t>(name, n_rows, n_cols);
            break;
        case datatype_t::S8:
            return this->template create_2d_socket_fwd<int8_t>(name, n_rows, n_cols);
            break;
        case datatype_t::U64:
            return this->template create_2d_socket_fwd<uint64_t>(name, n_rows, n_cols);
            break;
        case datatype_t::U32:
            return this->template create_2d_socket_fwd<uint32_t>(name, n_rows, n_cols);
            break;
        case datatype_t::U16:
            return this->template create_2d_socket_fwd<uint16_t>(name, n_rows, n_cols);
            break;
        case datatype_t::U8:
            return this->template create_2d_socket_fwd<uint8_t>(name, n_rows, n_cols);
            break;
        default:
        {
            std::stringstream message;
            message << "This should never happen.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            break;
        }
    }
}

void
Task::create_codelet(std::function<int(module::Module& m, Task& t, const size_t frame_id)>& codelet)
{
    this->codelet = codelet;

    // create automatically a socket that contains the status of the task
    const bool hack_status = true;
    auto s = this->template create_2d_socket_out<int>("status", 1, this->get_module().get_n_waves(), hack_status);
    this->sockets[s]->dataptr = (void*)this->status.data();
}

void
Task::update_n_frames(const size_t old_n_frames, const size_t new_n_frames)
{
    for (auto& s : this->sockets)
    {
        if (s->get_name() == "status")
        {
            if (this->get_module().get_n_waves() * sizeof(int) != s->get_databytes())
            {
                s->set_databytes(this->get_module().get_n_waves() * sizeof(int));
                this->status.resize(this->get_module().get_n_waves());
                s->set_dataptr((void*)this->status.data());
            }
        }
        else
        {
            const auto old_databytes = s->get_databytes();
            const auto new_databytes = (old_databytes / old_n_frames) * new_n_frames;
            s->set_databytes(new_databytes);

            const size_t prev_n_rows_wo_nfra = s->get_n_rows() / old_n_frames;
            s->set_n_rows(prev_n_rows_wo_nfra * new_n_frames);

            if (s->get_type() == socket_t::SOUT)
            {
                s->set_out_buffer(new_databytes);
            }
        }
    }
}

void
Task::update_n_frames_per_wave(const size_t /*old_n_frames_per_wave*/, const size_t new_n_frames_per_wave)
{
    size_t s_id = 0;
    for (auto& s : this->sockets)
    {
        if (s->get_name() == "status")
        {
            if (this->get_module().get_n_waves() * sizeof(int) != s->get_databytes())
            {
                s->set_databytes(this->get_module().get_n_waves() * sizeof(int));
                this->status.resize(this->get_module().get_n_waves());
                s->set_dataptr((void*)this->status.data());
            }
        }
        else
        {
            this->sockets_data[s_id].resize(
              (new_n_frames_per_wave > 1) ? this->sockets_databytes_per_frame[s_id] * new_n_frames_per_wave : 0);
        }
        s_id++;
    }
}

void
Task::allocate_outbuffers()
{
    if (!this->is_outbuffers_allocated())
    {
        std::function<void(Socket * socket, void* data_ptr)> spread_dataptr =
          [&spread_dataptr](Socket* socket, void* data_ptr)
        {
            for (auto bound_socket : socket->get_bound_sockets())
            {
                if (bound_socket->get_type() == socket_t::SIN)
                {
                    bound_socket->set_dataptr(data_ptr);
                }
                else if (bound_socket->get_type() == socket_t::SFWD)
                {
                    bound_socket->set_dataptr(data_ptr);
                    spread_dataptr(bound_socket, data_ptr);
                }
                else
                {
                    std::stringstream message;
                    message << "bound socket is of type SOUT, but should be SIN or SFWD";
                    throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
                }
            }
        };
        for (auto s : this->sockets)
        {
            if (s->get_type() == socket_t::SOUT && s->get_name() != "status")
            {
                if (s->get_dataptr() == nullptr)
                {
                    s->allocate_buffer();
                    spread_dataptr(s.get(), s->get_dataptr());
                }
            }
        }
        this->set_outbuffers_allocated(true);
    }
}
void
Task::deallocate_outbuffers()
{
    if (!this->is_outbuffers_allocated())
    {
        std::stringstream message;
        message << "Task out sockets buffers are not allocated"
                << ", task name : " << this->get_name();
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    std::function<void(Socket * socket)> spread_nullptr = [&spread_nullptr](Socket* socket)
    {
        for (auto bound_socket : socket->get_bound_sockets())
        {
            if (bound_socket->get_type() == socket_t::SIN)
            {
                bound_socket->set_dataptr(nullptr);
            }
            else if (bound_socket->get_type() == socket_t::SFWD)
            {
                bound_socket->set_dataptr(nullptr);
                spread_nullptr(bound_socket);
            }
            else
            {
                std::stringstream message;
                message << "bound socket is of type SOUT, but should be SIN or SFWD";
                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            }
        }
    };
    for (auto s : this->sockets)
    {
        if (s->get_type() == socket_t::SOUT && s->get_name() != "status")
        {
            if (s->get_dataptr() != nullptr)
            {
                s->deallocate_buffer();
                spread_nullptr(s.get());
            }
        }
    }
    this->set_outbuffers_allocated(false);
}

bool
Task::can_exec() const
{
    for (size_t i = 0; i < sockets.size(); i++)
        if (sockets[i]->dataptr == nullptr) return false;
    return true;
}

std::chrono::nanoseconds
Task::get_duration_total() const
{
    return this->duration_total;
}

std::chrono::nanoseconds
Task::get_duration_avg() const
{
    return this->duration_total / this->n_calls;
}

std::chrono::nanoseconds
Task::get_duration_min() const
{
    return this->duration_min;
}

std::chrono::nanoseconds
Task::get_duration_max() const
{
    return this->duration_max;
}

const std::vector<std::string>&
Task::get_timers_name() const
{
    return this->timers_name;
}

const std::vector<uint32_t>&
Task::get_timers_n_calls() const
{
    return this->timers_n_calls;
}

const std::vector<std::chrono::nanoseconds>&
Task::get_timers_total() const
{
    return this->timers_total;
}

const std::vector<std::chrono::nanoseconds>&
Task::get_timers_min() const
{
    return this->timers_min;
}

const std::vector<std::chrono::nanoseconds>&
Task::get_timers_max() const
{
    return this->timers_max;
}

size_t
Task::get_n_input_sockets() const
{
    return this->n_input_sockets;
}

size_t
Task::get_n_output_sockets() const
{
    return this->n_output_sockets;
}

size_t
Task::get_n_fwd_sockets() const
{
    return this->n_fwd_sockets;
}

void
Task::register_timer(const std::string& name)
{
    this->timers_name.push_back(name);
    this->timers_n_calls.push_back(0);
    this->timers_total.push_back(std::chrono::nanoseconds(0));
    this->timers_max.push_back(std::chrono::nanoseconds(0));
    this->timers_min.push_back(std::chrono::nanoseconds(0));
}

void
Task::reset()
{
    this->n_calls = 0;
    this->duration_total = std::chrono::nanoseconds(0);
    this->duration_min = std::chrono::nanoseconds(0);
    this->duration_max = std::chrono::nanoseconds(0);

    for (auto& x : this->timers_n_calls)
        x = 0;
    for (auto& x : this->timers_total)
        x = std::chrono::nanoseconds(0);
    for (auto& x : this->timers_min)
        x = std::chrono::nanoseconds(0);
    for (auto& x : this->timers_max)
        x = std::chrono::nanoseconds(0);
}

Task*
Task::clone() const
{
    Task* t = new Task(*this);
    t->sockets.clear();
    t->last_input_socket = nullptr;
    t->fake_input_sockets.clear();
    t->set_outbuffers_allocated(false);

    for (auto s : this->sockets)
    {
        void* dataptr = nullptr;
        if (s->get_type() == socket_t::SOUT)
        {
            if (s->get_name() == "status")
            {
                dataptr = (void*)t->status.data();
            }
        }
        else if (s->get_type() == socket_t::SIN || s->get_type() == socket_t::SFWD)
            dataptr = s->_get_dataptr();

        // No need to allocate memory when cloning
        const std::pair<size_t, size_t> databytes_per_dim = { s->get_n_rows(), s->get_databytes() / s->get_n_rows() };
        auto s_new = std::shared_ptr<Socket>(
          new Socket(*t, s->get_name(), s->get_datatype(), databytes_per_dim, s->get_type(), s->is_fast(), dataptr));
        t->sockets.push_back(s_new);

        if (s_new->get_type() == socket_t::SIN || s_new->get_type() == socket_t::SFWD)
            t->last_input_socket = s_new.get();
    }

    return t;
}

void
Task::_bind(Socket& s_out, const int priority)
{
    // check if the 's_out' socket is already used for an other fake input socket
    bool already_bound = false;
    for (auto& fsi : this->fake_input_sockets)
        if (&fsi->get_bound_socket() == &s_out)
        {
            already_bound = true;
            break;
        }

    // check if the 's_out' socket is already used for an other read input/fwd socket
    if (!already_bound)
        for (auto& s : this->sockets)
            if (s->get_type() == socket_t::SIN || s->get_type() == socket_t::SFWD)
            {
                try // because 's->get_bound_socket()' can throw if s->bound_socket == 'nullptr'
                {
                    if (&s->get_bound_socket() == &s_out)
                    {
                        already_bound = true;
                        break;
                    }
                }
                catch (...)
                {
                }
            }

    // if the s_out socket is not already bound, then create a new fake input socket
    if (!already_bound)
    {
        this->fake_input_sockets.push_back(
          std::shared_ptr<Socket>(new Socket(*this,
                                             "fake" + std::to_string(this->fake_input_sockets.size()),
                                             s_out.get_datatype(),
                                             s_out.get_databytes(),
                                             socket_t::SIN,
                                             this->is_fast())));
        this->fake_input_sockets.back()->_bind(s_out, priority);
        this->last_input_socket = this->fake_input_sockets.back().get();
        this->n_input_sockets++;
    }
}

void
Task::bind(Socket& s_out, const int priority)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Task::bind()' should be replaced by 'Task::operator='."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    this->_bind(s_out, priority);
}

void
Task::_bind(Task& t_out, const int priority)
{
    this->_bind(*t_out.sockets.back(), priority);
}

void
Task::bind(Task& t_out, const int priority)
{
#ifdef SPU_SHOW_DEPRECATED
    std::clog << rang::tag::warning << "Deprecated: 'Task::bind()' should be replaced by 'Task::operator='."
              << std::endl;
#ifdef SPU_STACKTRACE
#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    cpptrace::generate_trace().print(std::clog, enable_color);
#endif
#endif
    this->_bind(t_out, priority);
}

void
Task::operator=(Socket& s_out)
{
#ifndef SPU_FAST
    if (s_out.get_type() == socket_t::SOUT || s_out.get_type() == socket_t::SFWD)
#endif
        this->_bind(s_out);
#ifndef SPU_FAST
    else
    {
        std::stringstream message;
        message << "'s_out' should be and output socket ("
                << "'s_out.datatype' = " << type_to_string[s_out.get_datatype()] << ", "
                << "'s_out.name' = " << s_out.get_name() << ", "
                << "'s_out.task.name' = " << s_out.task.get_name() << ", "
                << "'s_out.type' = " << (s_out.get_type() == socket_t::SIN ? "SIN" : "SOUT") << ", "
                << "'s_out.task.module.name' = " << s_out.task.get_module().get_custom_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
#endif
}

void
Task::operator=(Task& t_out)
{
    (*this) = *t_out.sockets.back();
}

size_t
Task::unbind(Socket& s_out)
{
    if (this->fake_input_sockets.size())
    {
        size_t i = 0;
        for (auto& fsi : this->fake_input_sockets)
        {
            if (&fsi->get_bound_socket() == &s_out)
            {
                const auto pos = fsi->unbind(s_out);
                if (this->last_input_socket == fsi.get()) this->last_input_socket = nullptr;
                this->fake_input_sockets.erase(this->fake_input_sockets.begin() + i);
                this->n_input_sockets--;
                if (this->fake_input_sockets.size() && this->last_input_socket == nullptr)
                    this->last_input_socket = this->fake_input_sockets.back().get();
                return pos;
            }
            i++;
        }

        std::stringstream message;
        message << "'s_out' is not bound the this task ("
                << "'s_out.datatype' = " << type_to_string[s_out.datatype] << ", "
                << "'s_out.name' = " << s_out.get_name() << ", "
                << "'s_out.task.name' = " << s_out.task.get_name() << ", "
                << "'s_out.task.module.name' = " << s_out.task.get_module().get_custom_name() << ", "
                << "'task.name' = " << this->get_name() << ", "
                << "'task.module.name' = " << this->get_module().get_custom_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    else
    {
        std::stringstream message;
        message << "This task does not have fake input socket to unbind ("
                << "'s_out.datatype' = " << type_to_string[s_out.datatype] << ", "
                << "'s_out.name' = " << s_out.get_name() << ", "
                << "'s_out.task.name' = " << s_out.task.get_name() << ", "
                << "'s_out.task.module.name' = " << s_out.task.get_module().get_custom_name() << ", "
                << "'task.name' = " << this->get_name() << ", "
                << "'task.module.name' = " << this->get_module().get_custom_name() << ").";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
}

size_t
Task::unbind(Task& t_out)
{
    return this->unbind(*t_out.sockets.back());
}

size_t
Task::get_n_static_input_sockets() const
{
    size_t n = 0;
    for (auto& s : this->sockets)
        if (s->get_type() == socket_t::SIN && s->_get_dataptr() != nullptr && s->bound_socket == nullptr) n++;
    return n;
}

bool
Task::is_stateless() const
{
    return this->get_module().is_stateless();
}

bool
Task::is_stateful() const
{
    return this->get_module().is_stateful();
}

bool
Task::is_replicable() const
{
    return this->replicable;
}

void
Task::set_replicability(const bool replicable)
{
    if (replicable && !this->module->is_clonable())
    {
        std::stringstream message;
        message << "The replicability of this task cannot be set to true because its corresponding module is not "
                << "clonable (task.name = '" << this->get_name() << "', module.name = '"
                << this->get_module().get_name() << "').";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    this->replicable = replicable;
    // this->replicable = replicable ? this->module->is_clonable() : false;
}

void
Task::set_outbuffers_allocated(const bool outbuffers_allocated)
{
    this->outbuffers_allocated = outbuffers_allocated;
}

// ==================================================================================== explicit template instantiation
template size_t
Task::create_2d_socket_in<int8_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_in<uint8_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_in<int16_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_in<uint16_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_in<int32_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_in<uint32_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_in<int64_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_in<uint64_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_in<float>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_in<double>(const std::string&, const size_t, const size_t);

template size_t
Task::create_2d_socket_out<int8_t>(const std::string&, const size_t, const size_t, const bool);
template size_t
Task::create_2d_socket_out<uint8_t>(const std::string&, const size_t, const size_t, const bool);
template size_t
Task::create_2d_socket_out<int16_t>(const std::string&, const size_t, const size_t, const bool);
template size_t
Task::create_2d_socket_out<uint16_t>(const std::string&, const size_t, const size_t, const bool);
template size_t
Task::create_2d_socket_out<int32_t>(const std::string&, const size_t, const size_t, const bool);
template size_t
Task::create_2d_socket_out<uint32_t>(const std::string&, const size_t, const size_t, const bool);
template size_t
Task::create_2d_socket_out<int64_t>(const std::string&, const size_t, const size_t, const bool);
template size_t
Task::create_2d_socket_out<uint64_t>(const std::string&, const size_t, const size_t, const bool);
template size_t
Task::create_2d_socket_out<float>(const std::string&, const size_t, const size_t, const bool);
template size_t
Task::create_2d_socket_out<double>(const std::string&, const size_t, const size_t, const bool);

template size_t
Task::create_2d_socket_fwd<int8_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_fwd<uint8_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_fwd<int16_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_fwd<uint16_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_fwd<int32_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_fwd<uint32_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_fwd<int64_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_fwd<uint64_t>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_fwd<float>(const std::string&, const size_t, const size_t);
template size_t
Task::create_2d_socket_fwd<double>(const std::string&, const size_t, const size_t);
// ==================================================================================== explicit template instantiation
