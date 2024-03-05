/*
 * The following code is strongly inspired by the one from the cpptrace lib
 * - see: https://github.com/jeremy-rifkin/cpptrace/blob/main/docs/signal-safe-tracing.md
 */

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <chrono>

#include <cpptrace/cpptrace.hpp>

int main()
{
	std::cout << "aff3ct-core signal tracer -> running..." << std::endl;
	auto t_start = std::chrono::steady_clock::now();
	std::chrono::nanoseconds duration;
	cpptrace::object_trace trace;
	do
	{
		cpptrace::safe_object_frame frame;
		// fread used over read because a read() from a pipe might not read the full frame
		size_t res = fread(&frame, sizeof(frame), 1, stdin);
		if (res == 0)
		{
			break;
		}
		else if (res != 1)
		{
			std::cerr << "Something went wrong while reading from the pipe" << res << " " << std::endl;
			break;
		}
		else
		{
			trace.frames.push_back(frame.resolve());
		}
		duration = std::chrono::steady_clock::now() - t_start;
	}
	while ((size_t)duration.count() < 3e9); // 3 seconds timeout

#ifdef AFF3CT_CORE_COLORS
	bool enable_color = true;
#else
	bool enable_color = false;
#endif
	trace.resolve().print(std::cerr, enable_color);

	std::cout << "aff3ct-core signal tracer -> exit" << std::endl;

	return EXIT_SUCCESS;
}