/*!
 * \file
 * \brief Class module::Switcher.
 */
#ifndef SWITCHER_HPP_
#define SWITCHER_HPP_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <typeindex>
#include <vector>

#include "Module/Module.hpp"
#include "Runtime/Sequence/Sequence.hpp"
#include "Runtime/Socket/Socket.hpp"
#include "Runtime/Task/Task.hpp"
#include "Tools/Interface/Interface_reset.hpp"

namespace spu
{
namespace module
{
namespace swi
{
enum class tsk : size_t
{
    commute,
    select,
    SIZE
};
}

class Switcher
  : public Module
  , public tools::Interface_reset
{
    friend runtime::Sequence; // Sequence is friend to enable the no copy mode (0 copy)

  public:
    inline runtime::Task& operator[](const swi::tsk t);
    inline const runtime::Task& operator[](const swi::tsk t) const;
    inline runtime::Socket& operator[](const std::string& tsk_sck);

  protected:
    const size_t n_data_sockets;
    const size_t n_elmts_commute;
    const size_t n_elmts_select;
    const size_t n_bytes_commute;
    const size_t n_bytes_select;
    const std::type_index datatype_commute;
    const std::type_index datatype_select;
    size_t path;

    bool no_copy_commute;
    bool no_copy_select;

  public:
    inline Switcher(const size_t n_data_sockets,
                    const size_t n_elmts_commute,
                    const std::type_index datatype_commute,
                    const size_t n_elmts_select,
                    const std::type_index datatype_select);

    inline Switcher(const size_t n_data_sockets, const size_t n_elmts, const std::type_index datatype);
    virtual ~Switcher() = default;
    virtual Switcher* clone() const;

    inline size_t get_n_data_sockets() const;
    inline size_t get_n_elmts_commute() const;
    inline size_t get_n_elmts_select() const;
    inline size_t get_n_bytes_commute() const;
    inline size_t get_n_bytes_select() const;
    inline std::type_index get_datatype_commute() const;
    inline std::type_index get_datatype_select() const;
    inline size_t get_path() const;

    inline bool is_no_copy_commute() const;
    inline bool is_no_copy_select() const;

    virtual void reset();

  protected:
    inline void set_path(const size_t path);
    inline void set_no_copy_commute(const bool no_copy_commute);
    inline void set_no_copy_select(const bool no_copy_select);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Switcher/Switcher.hxx"
#endif

#endif /* SWITCHER_HPP_ */
