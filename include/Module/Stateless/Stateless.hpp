/*!
 * \file
 * \brief Class module::Stateless.
 */
#ifndef STATELESS_HPP_
#define STATELESS_HPP_

#include <cstdint>

#include "Runtime/Task/Task.hpp"
#include "Module/Module.hpp"

namespace aff3ct
{
namespace module
{

class Stateless : public Module
{
protected:
public:
	Stateless();
	virtual ~Stateless() = default;
	virtual Stateless* clone() const;

	void set_name(const std::string &name);
	void set_short_name(const std::string &short_name);
	runtime::Task& create_task(const std::string &name, const int id = -1);

	template <typename T>
	inline size_t create_socket_in(runtime::Task& task, const std::string &name, const size_t n_elmts);
	size_t create_socket_in(runtime::Task& task, const std::string &name, const size_t n_elmts, const std::type_index& datatype);

	template <typename T>
	inline size_t create_socket_out(runtime::Task& task, const std::string &name, const size_t n_elmts);
	size_t create_socket_out(runtime::Task& task, const std::string &name, const size_t n_elmts, const std::type_index& datatype);

	// Ajout de la création des sockets FWD pour les modules stateless 
	template <typename T>
	inline size_t create_socket_inout(runtime::Task& task, const std::string &name, const size_t n_elmts);
	size_t create_socket_inout(runtime::Task& task, const std::string &name, const size_t n_elmts, const std::type_index& datatype);

	void create_codelet(runtime::Task& task, std::function<int(Module &m, runtime::Task &t, const size_t frame_id)> codelet);
	void register_timer(runtime::Task& task, const std::string &key);
	virtual void set_n_frames_per_wave(const size_t n_frames_per_wave);
	void set_single_wave(const bool enable_single_wave);
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Stateless/Stateless.hxx"
#endif

#endif /* STATELESS_HPP_ */
