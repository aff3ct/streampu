#include "Tools/init.h"

#ifdef AFF3CT_CORE_STACKTRACE
/*
 * The following code is strongly inspired by the one from the cpptrace lib
 * - see: https://github.com/jeremy-rifkin/cpptrace/blob/main/docs/signal-safe-tracing.md
 */
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>

#include <cpptrace/cpptrace.hpp>

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

void do_signal_safe_trace(cpptrace::frame_ptr* buffer, std::size_t count)
{
	// setup pipe and spawn child
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
		execl("./bin/aff3ct-core-signal-tracer", "aff3ct-core-signal-tracer", nullptr);
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
}

void signal_handler(int signo, siginfo_t* info, void* context)
{
	 // print basic message
	char message[64];
	snprintf(message, sizeof(message), "[sig%s] occurred:\n", sys_signame[info->si_signo]);
	write(STDERR_FILENO, message, strlen(message));

	// generate trace
	constexpr std::size_t N = 100;
	cpptrace::frame_ptr buffer[N];
	std::size_t count = cpptrace::safe_generate_raw_trace(buffer, N);
	do_signal_safe_trace(buffer, count);

	// up to you if you want to exit or continue or whatever
	_exit(1);
}

void warmup_cpptrace()
{
	// This is done for any dynamic-loading shenanigans
	cpptrace::frame_ptr buffer[10];
	std::size_t count = cpptrace::safe_generate_raw_trace(buffer, 10);
	cpptrace::safe_object_frame frame;
	cpptrace::get_safe_object_frame(buffer[0], &frame);
}

int aff3ct::tools::init()
{
	warmup_cpptrace();

	// setup handler in the case of a segfault signal
	struct sigaction action;
	action.sa_flags = 0;
	action.sa_sigaction = &signal_handler;
	if (sigaction(SIGSEGV, &action, NULL) == -1) perror("sigaction SIGSEGV");
	if (sigaction(SIGINT,  &action, NULL) == -1) perror("sigaction SIGINT" );

	return 0;
}

#else

int aff3ct::tools::init()
{
	return 0;
}

#endif
