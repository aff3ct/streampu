#ifndef SIGNAL_HANDLER_HPP_
#define SIGNAL_HANDLER_HPP_

namespace aff3ct
{
namespace tools
{
class Signal_handler
{
public:
	static int init();
	static bool is_sigint();
	static bool is_sigsegv();
	static void reset_sigint();
	static void reset_sigsegv();
};
}
}

#endif /* SIGNAL_HANDLER_HPP_ */
