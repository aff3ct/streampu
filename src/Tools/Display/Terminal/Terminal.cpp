#include <csignal>
#include <cstdlib>

#include "Tools/Display/Terminal/Terminal.hpp"

using namespace aff3ct;
using namespace aff3ct::tools;

Terminal::Terminal()
  : stop_terminal(false)
  , t_term(std::chrono::steady_clock::now())
  , real_time_state(0)
{
}

Terminal::~Terminal()
{
    stop_temp_report(); // try to join the thread if this is not been done by the user
}

void
Terminal::temp_report(std::ostream& stream)
{
    this->report(stream, false);
}

void
Terminal::final_report(std::ostream& stream)
{
    this->stop_temp_report();
    this->report(stream, true);
    t_term = std::chrono::steady_clock::now();
}

void
Terminal::start_temp_report(const std::chrono::milliseconds freq)
{
    this->stop_temp_report();

    // launch a thread dedicated to the terminal display
    term_thread = std::thread(Terminal::start_thread_terminal, this, freq);

    t_term = std::chrono::steady_clock::now();
}

void
Terminal::stop_temp_report()
{
    if (term_thread.joinable())
    {
        stop_terminal = true;
        cond_terminal.notify_all();
        // wait the terminal thread to finish
        term_thread.join();
        stop_terminal = false;
    }

    real_time_state = 0;
}

void
Terminal::start_thread_terminal(Terminal* terminal, const std::chrono::milliseconds freq)
{
    const auto sleep_time = freq - std::chrono::milliseconds(0);
    while (!terminal->stop_terminal)
    {
        std::unique_lock<std::mutex> lock(terminal->mutex_terminal);
        if (terminal->cond_terminal.wait_for(lock, sleep_time) == std::cv_status::timeout)
            terminal->temp_report(std::clog); // display statistics in the terminal
    }
}
