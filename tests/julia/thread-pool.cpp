#include "Tools/Thread/Thread_pool/Julia/Thread_pool_Julia.hpp"
#include <cstdlib>
#include <ctime>
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
    printf("pid: %d\n", getpid()); // to kill if necessary

    option longopts[] = { { "threads", required_argument, NULL, 't' },
                          { "jluna-threads", required_argument, NULL, 'J' },
                          { "thread-pool", required_argument, NULL, 'V' },
                          { "print-stats", no_argument, NULL, 'p' },
                          { "verbose", no_argument, NULL, 'v' },
                          { "debug", no_argument, NULL, 'g' },
                          { "help", no_argument, NULL, 'h' },
                          { 0 } };

    size_t n_threads = std::thread::hardware_concurrency();
    bool print_stats = false;
    bool debug = false;
    bool verbose = false;
    bool jl_safe = false;
    int tp_version = 0;
    size_t jluna_threads = 0;

    while (1)
    {
        const int opt = getopt_long(argc, argv, "t:f:j:pbgvh", longopts, 0);
        if (opt == -1) break;
        switch (opt)
        {
            case 't':
                n_threads = atoi(optarg);
                break;
            case 'p':
                print_stats = true;
                break;
            case 'g':
                debug = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'V':
                tp_version = atoi(optarg);
                break;
            case 'J':
                jluna_threads = atoi(optarg);
                break;
            case 'h':
                std::cout << "usage: " << argv[0] << " [options]" << std::endl;
                std::cout << std::endl;
                std::cout << "  -t, --threads         "
                          << "Number of Julia threads                                               "
                          << "[" << n_threads << "]" << std::endl;
                std::cout << "  -J, --jluna-threads   "
                          << "Number of jluna threads                                               "
                          << "[" << jluna_threads << "]" << std::endl;
                std::cout << "  -V, --thread-pool     "
                          << "Version of the thread pool (0 for julia)                              "
                          << "[" << tp_version << "]" << std::endl;
                std::cout << "  -p, --print-stats     "
                          << "Enable to print per task statistics (performance will be reduced)     "
                          << "[" << (print_stats ? "true" : "false") << "]" << std::endl;
                std::cout << "  -v, --verbose         "
                          << "Enable verbose mode                                                   "
                          << "[" << (verbose ? "true" : "false") << "]" << std::endl;
                std::cout << "  -g, --debug           "
                          << "Enable task debug mode (print socket data)                            "
                          << "[" << (debug ? "true" : "false") << "]" << std::endl;
                std::cout << "  -h, --help            "
                          << "This help                                                             "
                          << "[false]" << std::endl;
                exit(0);
                break;
            default:
                break;
        }
    }

#ifdef SPU_JLUNA
    jluna::initialize(jluna_threads);
    gc_pause; // Macro
              // TODO: Remove this
#else
    jl_init();
#endif

    jl_function_t* print = jl_get_function(jl_base_module, "println");
    std::function<void(const char*)> print_verbose = [verbose](const char* msg)
    {
        if (verbose) std::cout << msg << std::endl;
    };

    std::function<void(const size_t)> func_exec = [](const size_t tid)
    {
        // jluna::Base["println"]("lambda called with ", tid);
        std::cout << "lambda called with " << tid << std::endl;
    };

    print_verbose("main -- 0");
    spu::tools::Thread_pool* tp;
#ifdef SPU_JLUNA
    switch (tp_version)
    {
        case 1:
            tp = new spu::tools::Thread_pool_Jluna_v1(n_threads);
            break;
        case 2:
            tp = new spu::tools::Thread_pool_Jluna_v2(n_threads);
            break;
        case 3:
            tp = new spu::tools::Thread_pool_Jluna_v3(n_threads);
            break;
        default:
            tp = new spu::tools::Thread_pool_Julia(n_threads);
            break;
    }
#else
    tp = new spu::tools::Thread_pool_Julia(n_threads);
#endif

    print_verbose("main -- 1");
    tp->init();
    print_verbose("main -- 2");

    jl_function_t* jl_get_threadid = jl_eval_string("return Base.Threads.threadid");
    jl_function_t* jl_println = jl_eval_string("return Base.println");
    std::function<void(const size_t)> func_exec1 = [jl_get_threadid, print_stats](const size_t tid)
    {
        time_t t0, t1;
        time(&t0);
        std::stringstream msg;
        msg << '[' << tid << "] COUCOUUUUUUUUUU 1 - jl_threadid = " << jl_unbox_int64(jl_call0(jl_get_threadid))
            << std::endl;
        std::cout << msg.str();
        time(&t1);
        if (print_stats)
        {
            std::stringstream ss;
            ss << '[' << tid << "] time taken for 2: " << t1 - t0 << std::endl;
            std::cout << ss.str();
        }
    };

    print_verbose("main -- 3");
    tp->run(func_exec1);
    print_verbose("main -- 4");

    (void)spu::tools::jl_eval_file("../tests/julia/computations.jl");
    jl_function_t* computation = jl_eval_string("return multiple_square_and_multiply");
    std::function<void(const size_t)> func_exec3 = [computation, jl_println, print_stats, verbose](const size_t tid)
    {
        time_t t0 = time(NULL);
        jl_value_t* threadid = jl_box_int32(tid);
        jl_value_t* jl_text3 = jl_eval_string("return \" - \"");
        jl_value_t* p = jl_box_int32(1392908813);
        jl_value_t* r = jl_call1(computation, p);
        int64_t res = jl_unbox_int64(r);
        if (verbose)
        {
            // jl_call3(jl_println, threadid, jl_text3, r);
            std::stringstream msg;
            msg << jl_unbox_int64(threadid) << " - " << jl_unbox_int64(r) << std::endl;
            std::cout << msg.str();
        }
        time_t t1 = time(NULL);
        if (print_stats)
        {
            std::stringstream ss;
            ss << '[' << tid << "] time taken for 2: " << t1 - t0 << std::endl;
            std::cout << ss.str();
        }
    };

    tp->run(func_exec3, true);
    print_verbose("main -- 5");
    tp->wait();
    print_verbose("main -- 6");
    delete tp;
    print_verbose("main -- 7");

#ifndef SPU_JLUNA
    jl_atexit_hook(0);
#endif // !SPU_JLUNA

    // Jluna sandbox
    // ==============================================================================================
    // END
    return 0;
}
