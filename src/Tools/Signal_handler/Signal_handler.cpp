/*
 * The following code is strongly inspired by the one from the cpptrace lib
 * - see: https://github.com/jeremy-rifkin/cpptrace/blob/main/docs/signal-safe-tracing.md
 */

#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "Runtime/Sequence/Sequence.hpp"
#include "Tools/Debug/hellgrind_annotations.h"
#include "Tools/Display/rang_format/rang_format.h"
#include "Tools/Signal_handler/Signal_handler.hpp"

using namespace spu;
using namespace spu::tools;

bool g_sigsegv = false;
bool g_sigint = false;

#ifdef SPU_STACKTRACE_SEGFAULT

#include <cstring>

#include <cpptrace/cpptrace.hpp>

#ifdef SPU_STACKTRACE_SEGFAULT_LIBUNWIND

#include <sys/wait.h>
#include <unistd.h>

// this is just a utility I like, it makes the pipe API more expressive.
struct pipe_t
{
    union
    {
        struct
        {
            int read_end;
            int write_end;
        };
        int data[2];
    };
};

#if defined(_WIN64) || defined(_WIN32)
void
signal_sigsegv_handler(int signo)
#else /* Unix-like */
void
signal_sigsegv_handler(int signo, siginfo_t* info, void* context)
#endif
{
    g_sigsegv = true;

    // print basic message
    std::cerr << rang::tag::error << "Signal \"Segmentation Violation\" caught by StreamPU signal handler!"
              << std::endl;
    std::cerr << rang::tag::error << "Printing stack trace (if possible) and then, exiting..." << std::endl;

    cpptrace::frame_ptr buffer[100];
    size_t count = cpptrace::safe_generate_raw_trace(buffer, 100);
    pipe_t input_pipe;
    pipe(input_pipe.data);
    const pid_t pid = fork();
    if (pid == -1)
        // some error occurred :-(
        return;

    if (pid == 0) // child
    {
        dup2(input_pipe.read_end, STDIN_FILENO);
        close(input_pipe.read_end);
        close(input_pipe.write_end);
        execlp("streampu-signal-tracer", "streampu-signal-tracer", nullptr);
        _exit(1);
    }

    // resolve to safe_object_frames and write those to the pipe
    for (size_t i = 0; i < count; i++)
    {
        cpptrace::safe_object_frame frame;
        cpptrace::get_safe_object_frame(buffer[i], &frame);
        write(input_pipe.write_end, &frame, sizeof(frame));
    }
    close(input_pipe.read_end);
    close(input_pipe.write_end);

    // wait for child
    waitpid(pid, nullptr, 0);

    _exit(1);
}

#else /* SPU_STACKTRACE_SEGFAULT_LIBUNWIND */

// /!\ UNSAFE METHOD
#if defined(_WIN64) || defined(_WIN32)
void
signal_sigsegv_handler(int signo)
#else /* Unix-like */
void
signal_sigsegv_handler(int signo, siginfo_t* info, void* context)
#endif
{
    g_sigsegv = true;

    // print basic message
    std::cerr << rang::tag::error << "Signal \"Segmentation Violation\" caught by StreamPU signal handler!"
              << std::endl;
    std::cerr << rang::tag::error << "Printing stack trace (if possible) and then, exiting..." << std::endl;

#ifdef SPU_COLORS
    bool enable_color = true;
#else
    bool enable_color = false;
#endif
    // according to the documentation, calling 'cpptrace::generate_trace()' in a signal handler is unsafe and may lead
    // to deadlock or memory corruption...
    cpptrace::generate_trace().print(std::cerr, enable_color);

    std::exit(1);
}

#endif /* SPU_STACKTRACE_SEGFAULT_LIBUNWIND */

void
warmup_cpptrace()
{
    // This is done for any dynamic-loading shenanigans
    cpptrace::frame_ptr buffer[10];
    cpptrace::safe_generate_raw_trace(buffer, 10);
    cpptrace::safe_object_frame frame;
    cpptrace::get_safe_object_frame(buffer[0], &frame);
}

#endif /* SPU_STACKTRACE_SEGFAULT */

bool g_is_interrupt = false;
#if defined(_WIN64) || defined(_WIN32)
void
signal_sigint_handler(int /*signo*/)
#else /* Unix-like */
void
signal_sigint_handler(int /*signo*/, siginfo_t* /*info*/, void* /*context*/)
#endif
{
    if (g_is_interrupt)
    {
        // print basic message
        std::cerr << std::endl
                  << rang::tag::error << "Signal \"Interruption\" caught twice by StreamPU signal handler!"
                  << std::endl;
        std::cerr << rang::tag::error << "Killing the application RIGHT NOW sir!" << std::endl;
        exit(1);
    }
    else
    {
        g_is_interrupt = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(250)); // 250 ms of passive waiting

        // print basic message
        std::clog << std::endl
                  << rang::tag::info << "Signal \"Interruption\" caught by StreamPU signal handler!" << std::endl;
        std::clog << rang::tag::info << "Stopping Sequence::exec() and/or Pipeline::exec()..." << std::endl;

        g_sigint = true;

        g_is_interrupt = false;
    }
}

int
Signal_handler::init()
{
// SIGSEGV handler
#ifdef SPU_STACKTRACE_SEGFAULT
    cpptrace::absorb_trace_exceptions(false);
    // cpptrace::register_terminate_handler();
    warmup_cpptrace();

#if defined(_WIN64) || defined(_WIN32)
    signal(SIGSEGV, signal_sigsegv_handler);
#else
    struct sigaction action_sigsegv = {};
    action_sigsegv.sa_flags = 0;
    action_sigsegv.sa_sigaction = &signal_sigsegv_handler;
    if (sigaction(SIGSEGV, &action_sigsegv, NULL) == -1) perror("sigaction SIGSEGV");
#endif /* defined(_WIN64) || defined(_WIN32) */
#endif /* SPU_STACKTRACE_SEGFAULT */

// SIGINT handler
#if defined(_WIN64) || defined(_WIN32)
    signal(SIGINT, signal_sigint_handler);
#else
    struct sigaction action_sigint = {};
    action_sigint.sa_flags = 0;
    action_sigint.sa_sigaction = &signal_sigint_handler;
    if (sigaction(SIGINT, &action_sigint, NULL) == -1) perror("sigaction SIGINT");
#endif /* defined(_WIN64) || defined(_WIN32) */

    return 0;
}

bool
Signal_handler::is_sigint()
{
    ANNOTATE_HAPPENS_AFTER(&g_sigint);
    auto ret = g_sigint;
    ANNOTATE_HAPPENS_BEFORE(&g_sigint);
    return ret;
}

void
Signal_handler::reset_sigint()
{
    ANNOTATE_HAPPENS_AFTER(&g_sigint);
    g_sigint = false;
    ANNOTATE_HAPPENS_BEFORE(&g_sigint);
}

bool
Signal_handler::is_sigsegv()
{
    ANNOTATE_HAPPENS_AFTER(&g_sigsegv);
    auto ret = g_sigsegv;
    ANNOTATE_HAPPENS_BEFORE(&g_sigsegv);
    return ret;
}

void
Signal_handler::reset_sigsegv()
{
    ANNOTATE_HAPPENS_AFTER(&g_sigsegv);
    g_sigsegv = false;
    ANNOTATE_HAPPENS_BEFORE(&g_sigsegv);
}
