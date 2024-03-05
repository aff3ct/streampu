#include <cstdio>
#include <iostream>
#include <unistd.h>

#include <cpptrace/cpptrace.hpp>

int main()
{
	std::cout << "aff3ct-core signal tracer is running..." << std::endl;
	cpptrace::object_trace trace;
	while(true)
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
	}
	trace.resolve().print();

	return EXIT_SUCCESS;
}