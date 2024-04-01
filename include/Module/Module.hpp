/*!
 * \file
 * \brief Class module::Module.
 */
#ifndef MODULE_HPP_
#define MODULE_HPP_

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "Runtime/Socket/Socket.hpp"
#include "Runtime/Task/Task.hpp"
#include "Tools/Interface/Interface_clone.hpp"
#include "Tools/Interface/Interface_get_set_n_frames.hpp"

namespace aff3ct
{
namespace module
{
/*!
 * \class Module
 *
 * \brief A Module is an abstract concept. Basically, all the objects used in a Simulation are a Module.
 */
class Module
  : public tools::Interface_clone
  , public tools::Interface_get_set_n_frames
{
  protected:
    size_t n_frames; /*!< Number of frames to process in this Module */
    size_t n_frames_per_wave;
    size_t n_waves;
    size_t n_frames_per_wave_rest;
    bool single_wave;
    std::string name;        /*!< Name of the Module. */
    std::string short_name;  /*!< Short name of the Module. */
    std::string custom_name; /*!< Custom name of the Module. */
    std::vector<std::shared_ptr<runtime::Task>> tasks_with_nullptr;

  public:
    std::vector<std::shared_ptr<runtime::Task>> tasks;

    /*!
     * \brief Constructor.
     */
    explicit Module();

    /*!
     * \brief Destructor.
     */
    virtual ~Module() = default;

    virtual Module* clone() const;

    /*!
     * \brief Get the number of frames.
     *
     * \return the number of frames to process in this Module.
     */
    inline size_t get_n_frames() const;

    virtual void set_n_frames(const size_t n_frames);

    inline size_t get_n_frames_per_wave() const;

    inline size_t get_n_waves() const;

    inline size_t get_n_frames_per_wave_rest() const;

    inline bool is_single_wave() const;

    const std::string& get_name() const;

    const std::string& get_short_name() const;

    void set_custom_name(const std::string& custom_name);

    const std::string& get_custom_name() const;

    void remove_custom_name();

    inline runtime::Task& operator[](const size_t id);
    inline const runtime::Task& operator[](const size_t id) const;
    inline runtime::Socket& operator[](const std::string& tsk_sck);
    inline runtime::Task& operator()(const std::string& tsk_name);

    void set_fast(const bool fast);

    void create_reset_task();

  protected:
    void deep_copy(const Module& m);

    void set_name(const std::string& name);

    void set_short_name(const std::string& short_name);

    runtime::Task& create_task(const std::string& name, const int id = -1);
    runtime::Task& create_tsk(const std::string& name, const int id = -1);

    template<typename T>
    size_t create_socket_in(runtime::Task& task, const std::string& name, const size_t n_elmts);
    size_t create_socket_in(runtime::Task& task,
                            const std::string& name,
                            const size_t n_elmts,
                            const std::type_index& datatype);
    size_t create_socket_in(runtime::Task& task,
                            const std::string& name,
                            const size_t n_elmts,
                            const runtime::datatype_t datatype);

    template<typename T>
    size_t create_sck_in(runtime::Task& task, const std::string& name, const size_t n_elmts);
    size_t create_sck_in(runtime::Task& task,
                         const std::string& name,
                         const size_t n_elmts,
                         const std::type_index& datatype);
    size_t create_sck_in(runtime::Task& task,
                         const std::string& name,
                         const size_t n_elmts,
                         const runtime::datatype_t datatype);

    template<typename T>
    size_t create_socket_out(runtime::Task& task, const std::string& name, const size_t n_elmts);
    size_t create_socket_out(runtime::Task& task,
                             const std::string& name,
                             const size_t n_elmts,
                             const std::type_index& datatype);
    size_t create_socket_out(runtime::Task& task,
                             const std::string& name,
                             const size_t n_elmts,
                             const runtime::datatype_t datatype);

    template<typename T>
    size_t create_sck_out(runtime::Task& task, const std::string& name, const size_t n_elmts);
    size_t create_sck_out(runtime::Task& task,
                          const std::string& name,
                          const size_t n_elmts,
                          const std::type_index& datatype);
    size_t create_sck_out(runtime::Task& task,
                          const std::string& name,
                          const size_t n_elmts,
                          const runtime::datatype_t datatype);

    template<typename T>
    size_t create_socket_fwd(runtime::Task& task, const std::string& name, const size_t n_elmts);
    size_t create_socket_fwd(runtime::Task& task,
                             const std::string& name,
                             const size_t n_elmts,
                             const std::type_index& datatype);
    size_t create_socket_fwd(runtime::Task& task,
                             const std::string& name,
                             const size_t n_elmts,
                             const runtime::datatype_t datatype);

    template<typename T>
    size_t create_sck_fwd(runtime::Task& task, const std::string& name, const size_t n_elmts);
    size_t create_sck_fwd(runtime::Task& task,
                          const std::string& name,
                          const size_t n_elmts,
                          const std::type_index& datatype);
    size_t create_sck_fwd(runtime::Task& task,
                          const std::string& name,
                          const size_t n_elmts,
                          const runtime::datatype_t datatype);

    template<typename T>
    size_t create_2d_socket_in(runtime::Task& task, const std::string& name, const size_t n_rows, const size_t n_cols);
    size_t create_2d_socket_in(runtime::Task& task,
                               const std::string& name,
                               const size_t n_rows,
                               const size_t n_cols,
                               const std::type_index& datatype);
    size_t create_2d_socket_in(runtime::Task& task,
                               const std::string& name,
                               const size_t n_rows,
                               const size_t n_cols,
                               const runtime::datatype_t datatype);

    template<typename T>
    size_t create_2d_sck_in(runtime::Task& task, const std::string& name, const size_t n_rows, const size_t n_cols);
    size_t create_2d_sck_in(runtime::Task& task,
                            const std::string& name,
                            const size_t n_rows,
                            const size_t n_cols,
                            const std::type_index& datatype);
    size_t create_2d_sck_in(runtime::Task& task,
                            const std::string& name,
                            const size_t n_rows,
                            const size_t n_cols,
                            const runtime::datatype_t datatype);

    template<typename T>
    size_t create_2d_socket_out(runtime::Task& task, const std::string& name, const size_t n_rows, const size_t n_cols);
    size_t create_2d_socket_out(runtime::Task& task,
                                const std::string& name,
                                const size_t n_rows,
                                const size_t n_cols,
                                const std::type_index& datatype);
    size_t create_2d_socket_out(runtime::Task& task,
                                const std::string& name,
                                const size_t n_rows,
                                const size_t n_cols,
                                const runtime::datatype_t datatype);

    template<typename T>
    size_t create_2d_sck_out(runtime::Task& task, const std::string& name, const size_t n_rows, const size_t n_cols);
    size_t create_2d_sck_out(runtime::Task& task,
                             const std::string& name,
                             const size_t n_rows,
                             const size_t n_cols,
                             const std::type_index& datatype);
    size_t create_2d_sck_out(runtime::Task& task,
                             const std::string& name,
                             const size_t n_rows,
                             const size_t n_cols,
                             const runtime::datatype_t datatype);

    template<typename T>
    size_t create_2d_socket_fwd(runtime::Task& task, const std::string& name, const size_t n_rows, const size_t n_cols);
    size_t create_2d_socket_fwd(runtime::Task& task,
                                const std::string& name,
                                const size_t n_rows,
                                const size_t n_cols,
                                const std::type_index& datatype);
    size_t create_2d_socket_fwd(runtime::Task& task,
                                const std::string& name,
                                const size_t n_rows,
                                const size_t n_cols,
                                const runtime::datatype_t datatype);

    template<typename T>
    size_t create_2d_sck_fwd(runtime::Task& task, const std::string& name, const size_t n_rows, const size_t n_cols);
    size_t create_2d_sck_fwd(runtime::Task& task,
                             const std::string& name,
                             const size_t n_rows,
                             const size_t n_cols,
                             const std::type_index& datatype);
    size_t create_2d_sck_fwd(runtime::Task& task,
                             const std::string& name,
                             const size_t n_rows,
                             const size_t n_cols,
                             const runtime::datatype_t datatype);

    void create_codelet(runtime::Task& task,
                        std::function<int(Module& m, runtime::Task& t, const size_t frame_id)> codelet);
    void create_cdl(runtime::Task& task,
                    std::function<int(Module& m, runtime::Task& t, const size_t frame_id)> codelet);

    void register_timer(runtime::Task& task, const std::string& key);

    virtual void set_n_frames_per_wave(const size_t n_frames_per_wave);

    void set_single_wave(const bool enable_single_wave);

  private:
    void _set_n_frames_per_wave(const size_t n_frames_per_wave);
    void _set_n_frames(const size_t n_frames);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Module.hxx"
#endif

#endif /* MODULE_HPP_ */
