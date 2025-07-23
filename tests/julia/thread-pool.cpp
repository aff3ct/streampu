#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <julia.h>
#include <sstream>
#include <stdio.h>
#include <unistd.h>

#include <streampu.hpp>

int
main(int argc, char** argv)
{
    option longopts[] = {
        { "threads", required_argument, NULL, 't' }, { "n-inter-frames", required_argument, NULL, 'f' },
        { "print-stats", no_argument, NULL, 'p' },   { "step-by-step", no_argument, NULL, 'b' },
        { "debug", no_argument, NULL, 'g' },         { "verbose", no_argument, NULL, 'v' },
        { "help", no_argument, NULL, 'h' },          { 0 }
    };

    size_t n_threads = std::thread::hardware_concurrency();
    size_t n_inter_frames = 1;
    bool no_copy_mode = true;
    bool print_stats = false;
    bool step_by_step = false;
    bool debug = false;
    bool verbose = false;
    bool jl_safe = false;

    while (1)
    {
        const int opt = getopt_long(argc, argv, "t:f:j:pbgvh", longopts, 0);
        if (opt == -1) break;
        switch (opt)
        {
            case 't':
                n_threads = atoi(optarg);
                break;
            case 'f':
                n_inter_frames = atoi(optarg);
                break;
            case 'c':
                no_copy_mode = false;
                break;
            case 'p':
                print_stats = true;
                break;
            case 'b':
                step_by_step = true;
                break;
            case 'g':
                debug = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
                std::cout << "usage: " << argv[0] << " [options]" << std::endl;
                std::cout << std::endl;
                std::cout << "  -t, --threads         "
                          << "Number of Julia threads                                               "
                          << "[" << n_threads << "]" << std::endl;
                std::cout << "  -f, --n-inter-frames  "
                          << "Number of frames to process in one task                               "
                          << "[" << n_inter_frames << "]" << std::endl;
                std::cout << "  -c, --copy-mode       "
                          << "Enable to copy data in sequence (performance will be reduced)         "
                          << "[" << (no_copy_mode ? "false" : "true") << "]" << std::endl;
                std::cout << "  -b, --step-by-step    "
                          << "Enable step-by-step sequence execution (performance will be reduced)  "
                          << "[" << (step_by_step ? "true" : "false") << "]" << std::endl;
                std::cout << "  -p, --print-stats     "
                          << "Enable to print per task statistics (performance will be reduced)     "
                          << "[" << (print_stats ? "true" : "false") << "]" << std::endl;
                std::cout << "  -g, --debug           "
                          << "Enable task debug mode (print socket data)                            "
                          << "[" << (debug ? "true" : "false") << "]" << std::endl;
                std::cout << "  -v, --verbose         "
                          << "Enable verbose mode                                                   "
                          << "[" << (verbose ? "true" : "false") << "]" << std::endl;
                std::cout << "  -h, --help            "
                          << "This help                                                             "
                          << "[false]" << std::endl;
                exit(0);
                break;
            default:
                break;
        }
    }

    printf("pid: %d\n", getpid()); // to kill if necessary
#ifdef SPU_JLUNA
    jluna::initialize(0);
#else
    jl_init();
#endif
    // gc_pause; // Macro
    //           // TODO: Remove this
    std::function<void(const size_t)> func_exec = [](const size_t tid)
    {
        // jluna::Base["println"]("lambda called with ", tid);
        std::cout << "lambda called with " << tid << std::endl;
    };

    std::cout << "main -- 0" << std::endl;
    spu::tools::Thread_pool_Julia tp(n_threads);
    std::cout << "main -- 1" << std::endl;
    tp.init();
    std::cout << "main -- 2" << std::endl;
    jl_function_t* jl_get_threadid = jl_eval_string("return Base.Threads.threadid");
    jl_function_t* jl_println = jl_eval_string("return Base.println");
    std::function<void(const size_t)> func_exec1 = [jl_get_threadid](const size_t tid)
    {
        std::stringstream msg;
        msg << '[' << tid << "] COUCOUUUUUUUUUU 1 - jl_threadid = " << jl_unbox_int64(jl_call0(jl_get_threadid))
            << std::endl;
        std::cout << msg.str();
    };
    std::cout << "main -- 3" << std::endl;
    tp.run(func_exec1);
    std::cout << "main -- 4" << std::endl;
    jl_function_t* computation = jl_eval_string(R""""(return function(p)
                                                        r::Int64 = 0
                                                        for m=1:(1 << 6)
                                                          x::UInt64 = 1
                                                          for i=1:32
                                                            x = (x * x) % p
                                                            x = (x * m) % p
                                                          end
                                                          r = r + x
                                                          if(r > p)
                                                            r = r - p
                                                          end
                                                        end
                                                        return Int32(r)
                                                      end
                                                      )"""");
    std::function<void(const size_t)> func_exec3 = [computation, jl_println](const size_t tid)
    {
        jl_value_t* threadid = jl_box_int32(tid);
        jl_value_t* jl_text3 = jl_eval_string("return \" - \"");
        jl_value_t* p = jl_box_int32(1392908813);
        jl_value_t* r = jl_call1(computation, p);
        int64_t res = jl_unbox_int64(r);
        // jl_call3(jl_println, threadid, jl_text3, r);
        std::stringstream msg;
        msg << jl_unbox_int64(threadid) << " - " << jl_unbox_int64(r) << std::endl;
        std::cout << msg.str();
    };
    tp.run(func_exec3, true);
    std::cout << "main -- 5" << std::endl;
    tp.wait();
    std::cout << "main -- 6" << std::endl;

#ifndef SPU_JLUNA
    jl_atexit_hook(0);
#endif // !SPU_JLUNA

    // Jluna sandbox
    // ==============================================================================================
    // END
    return 0;
}
