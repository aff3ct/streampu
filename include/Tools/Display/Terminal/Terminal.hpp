/*!
 * \file
 * \brief Class tools::Terminal.
 */
#ifndef TERMINAL_HPP_
#define TERMINAL_HPP_

#include <condition_variable>
#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>

#include "Tools/Reporter/Reporter.hpp"

namespace aff3ct
{
namespace tools
{
/*!
 * \class Terminal
 *
 * \brief The terminal display.
 */
class Terminal
{
private:
	std::thread term_thread;
	std::mutex mutex_terminal;
	std::condition_variable cond_terminal;
	bool stop_terminal;

protected:
	std::chrono::time_point<std::chrono::steady_clock> t_term;
	uint8_t real_time_state;

public:
	/*!
	 * \brief Constructor.
	 */
	Terminal();

	/*!
	 * \brief Destructor.
	 */
	virtual ~Terminal();

	/*!
	 * \brief Displays the terminal legend.
	 *
	 * \param stream: the stream to print the legend.
	 */
	virtual void legend(std::ostream &stream = std::cout) const = 0;

	/*!
	 * \brief Temporary report.
	 *
	 * \param stream: the stream to print the report.
	 */
	void temp_report(std::ostream &stream = std::cout);

	/*!
	 * \brief Final report.
	 *
	 * \param stream: the stream to print the report.
	 */
	void final_report(std::ostream &stream = std::cout);

	void start_temp_report(const std::chrono::milliseconds freq = std::chrono::milliseconds(500));

	void stop_temp_report();

protected:
	virtual void report(std::ostream &stream = std::cout, bool final = false) = 0;

private:
	static void start_thread_terminal(Terminal *terminal, const std::chrono::milliseconds freq);
};
}
}

#endif /* TERMINAL_HPP_ */
