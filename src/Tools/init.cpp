/*
 * The following code is strongly inspired by the one from the cpptrace lib
 * - see: https://github.com/jeremy-rifkin/cpptrace/blob/main/docs/signal-safe-tracing.md
 */

#include "Tools/init.h"

#ifdef AFF3CT_CORE_STACKTRACE_SEGFAULT

#include <iostream>
#include <csignal>

#include <cpptrace/cpptrace.hpp>

#ifdef AFF3CT_CORE_STACKTRACE_SEGFAULT_LIBUNWIND

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

// this is just a utility I like, it makes the pipe API more expressive.
struct pipe_t {
	union {
		struct {
			int read_end;
			int write_end;
		};
		int data[2];
	};
};

void signal_handler(int signo, siginfo_t* info, void* context)
{
	// print basic message
	char message[64];
	snprintf(message, sizeof(message), "Signal \"%s\" occurred:\n", strsignal(info->si_signo));
	write(STDERR_FILENO, message, strlen(message));

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
		execlp("aff3ct-core-signal-tracer", "aff3ct-core-signal-tracer", nullptr);
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

	// Wait for child
	waitpid(pid, nullptr, 0);

	_exit(1);
}

#else /* AFF3CT_CORE_STACKTRACE_SEGFAULT_LIBUNWIND */

// /!\ UNSAFE METHOD
void signal_handler(int signo, siginfo_t* info, void* context)
{
	// print basic message
	std::cout << "Signal \"" <<  strsignal(info->si_signo) << "\" occurred:" << std::endl;

#ifdef AFF3CT_CORE_COLORS
	bool enable_color = true;
#else
	bool enable_color = false;
#endif
	// according to the documentation, calling 'cpptrace::generate_trace()' in a signal handler is unsafe and may lead
	// to deadlock or memory corruption...
	cpptrace::generate_trace().print(std::cerr, enable_color);

	std::exit(1);
}

#endif /* AFF3CT_CORE_STACKTRACE_SEGFAULT_LIBUNWIND */

void warmup_cpptrace()
{
	// This is done for any dynamic-loading shenanigans
	cpptrace::frame_ptr buffer[10];
	cpptrace::safe_generate_raw_trace(buffer, 10);
	cpptrace::safe_object_frame frame;
	cpptrace::get_safe_object_frame(buffer[0], &frame);
}

int aff3ct::tools::init()
{
	cpptrace::absorb_trace_exceptions(false);
	// cpptrace::register_terminate_handler();
	warmup_cpptrace();

	// setup handler in the case of a segfault signal
	struct sigaction action;
	action.sa_flags = 0;
	action.sa_sigaction = &signal_handler;
	if (sigaction(SIGSEGV, &action, NULL) == -1) perror("sigaction SIGSEGV");
	if (sigaction(SIGINT,  &action, NULL) == -1) perror("sigaction SIGINT" );

	return 0;
}

#else /* AFF3CT_CORE_STACKTRACE_SEGFAULT */

int aff3ct::tools::init()
{
	return 0;
}

#endif /* AFF3CT_CORE_STACKTRACE_SEGFAULT */
