#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <getopt.h>
#include <iostream>
#include <julia.h>
#include <julia_threads.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <streampu.hpp>

// murmur3 64 bits finalizer
// c.f. aappleby/smhasher
uint64_t
murmur64(uint64_t k)
{
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdull;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ull;
    k ^= k >> 33;

    return k;
}

uint64_t
computation1(size_t tid, std::vector<uint64_t> array)
{
    uint64_t acc = murmur64(tid);

    for (auto x : array)
    {
        uint64_t h = murmur64(x);
        acc = (acc * h) ^ acc ^ (h << 16) ^ (h >> 48);
    }
    return acc;
}

int
main(int argc, char** argv)
{
    printf("pid: %d\n", getpid()); // to kill if necessary

    option longopts[] = { { "threads", required_argument, NULL, 't' },
                          { "jluna-threads", required_argument, NULL, 'J' },
                          { "thread-pool", required_argument, NULL, 'V' },
                          { "computation", required_argument, NULL, 'c' }, // TODO
                          { "seed", required_argument, NULL, 's' },
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
    size_t computation_id = 0;
    std::hash<std::string> hash;
    unsigned seed = 0;

    while (1)
    {
        const int opt = getopt_long(argc, argv, "t:J:V:c:s:pvh", longopts, 0);
        if (opt == -1) break;
        switch (opt)
        {
            case 't':
                n_threads = atoi(optarg);
                break;
            case 'J':
                jluna_threads = atoi(optarg);
                break;
            case 'c':
                computation_id = atoi(optarg);
                break;
            case 's':
                seed = hash(optarg);
                break;
            case 'p':
                print_stats = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'g':
                debug = true;
                break;
            case 'V':
                tp_version = atoi(optarg);
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
                std::cout << "  -c, --computation     "
                          << "Name of the julia function                                            "
                          << "[" << computation_id << "]" << std::endl;
                std::cout << "  -s, --seed            "
                          << "Seed for the computation (hash of a string)                           "
                          << "[" << seed << "]" << std::endl;
                std::cout << "  -p, --print-stats     "
                          << "Enable to print per task statistics (performance will be reduced)     "
                          << "[" << (print_stats ? "true" : "false") << "]" << std::endl;
                std::cout << "  -v, --verbose         "
                          << "Enable verbose mode                                                   "
                          << "[" << (verbose ? "true" : "false") << "]" << std::endl;
                // std::cout << "  -g, --debug           "
                //           << "Enable task debug mode (print socket data)                            "
                //           << "[" << (debug ? "true" : "false") << "]" << std::endl;
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

    auto t0 = std::chrono::steady_clock::now();
    std::chrono::nanoseconds duration;

    jl_function_t* print = jl_get_function(jl_base_module, "println");
    std::function<void(const char*)> print_verbose = [verbose](const char* msg)
    {
        if (verbose) std::cout << msg << std::endl;
    };

    std::vector<uint64_t> array(1 << 20);
    srand(seed);
    for (size_t i = 0; i < array.size(); i++)
    {
        array[i] = rand();
    }
    jl_value_t* datatype = jl_apply_array_type((jl_value_t*)jl_uint64_type, 1);
    jl_array_t* jl_array = jl_ptr_to_array_1d(datatype, array.data(), array.size(), 0);

    jl_function_t* jl_get_threadid = jl_eval_string("return Base.Threads.threadid");
    jl_function_t* jl_println = jl_eval_string("return Base.println");
    (void)spu::tools::jl_eval_file("../tests/julia/computations.jl");

    std::function<void(const size_t)> func_exec1 = [jl_get_threadid, print_stats](const size_t tid)
    {
        auto t0 = std::chrono::steady_clock::now();
        std::stringstream msg;
        msg << '[' << tid << "] COUCOUUUUUUUUUU 1 - jl_threadid = " << jl_unbox_int64(jl_call0(jl_get_threadid))
            << std::endl;
        std::cout << msg.str();
        std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t0;
        if (print_stats)
        {
            std::stringstream ss;
            ss << '[' << tid << "] time elapsed for 1: " << duration.count() / 1000000.0 << "ms" << std::endl;
            std::cout << ss.str();
        }
    };

    // jl_function_t* computation = jl_eval_string("return multiple_square_and_multiply");
    jl_function_t* computation = jl_eval_string("return computation1");
    std::function<void(std::vector<uint64_t>&, int, const size_t)> func_exec =
      [computation, jl_array, jl_println, print_stats, verbose](std::vector<uint64_t>& res, int i, const size_t tid)
    {
        if (verbose)
        {
            std::cout << "lambda " << i << " called with " << tid << std::endl;
        }
        auto t0 = std::chrono::steady_clock::now();
        jl_value_t* threadid = jl_box_int32(tid);
        jl_value_t* jl_text3 = jl_eval_string("return \" - \"");
        jl_value_t* r = jl_call2(computation, threadid, (jl_value_t*)jl_array);
        res[tid] = jl_unbox_uint64(r);
        std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t0;
        if (verbose)
        {
            std::stringstream msg;
            msg << jl_unbox_int64(threadid) << " - " << res[tid] << std::endl;
            std::cout << msg.str();
        }
        if (print_stats)
        {
            std::stringstream ss;
            ss << '[' << tid << "] time elapsed for 2: " << duration.count() / 1000000.0 << "ms" << std::endl;
            std::cout << ss.str();
        }
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

    if (verbose)
    {
        t0 = std::chrono::steady_clock::now();
        tp->run(func_exec1);
        duration = std::chrono::steady_clock::now() - t0;
        if (print_stats)
        {
            std::stringstream ss;
            ss << "[main] time elapsed for 0: " << duration.count() / 1000000.0 << "ms" << std::endl;
            std::cout << ss.str();
        }
    }

    print_verbose("main -- 3");

    std::vector<uint64_t> res1(n_threads);
    std::function<void(const size_t)> func_exec2 = [&func_exec, &res1](const size_t tid) { func_exec(res1, 1, tid); };

    t0 = std::chrono::steady_clock::now();
    tp->run(func_exec2);
    duration = std::chrono::steady_clock::now() - t0;
    if (print_stats)
    {
        std::stringstream ss;
        ss << "[main] time elapsed for 1: " << duration.count() / 1000000.0 << "ms" << std::endl;
        std::cout << ss.str();
    }

    print_verbose("main -- 4");

    std::vector<uint64_t> res2(n_threads);
    std::function<void(const size_t)> func_exec3 = [&func_exec, &res2](const size_t tid) { func_exec(res2, 2, tid); };

    t0 = std::chrono::steady_clock::now();
    tp->run(func_exec3, true);

    print_verbose("main -- 5");

    tp->wait();
    duration = std::chrono::steady_clock::now() - t0;
    if (print_stats)
    {
        std::stringstream ss;
        ss << "[main] time elapsed for 2: " << duration.count() / 1000000.0 << "ms" << std::endl;
        std::cout << ss.str();
    }

    print_verbose("main -- 6");

    std::vector<uint64_t> ref(n_threads);
    std::function<void(const size_t)> func_ref = [print_stats, verbose, &ref, &array](size_t tid)
    {
        auto t0 = std::chrono::steady_clock::now();
        ref[tid] = computation1(tid, array);

        if (verbose)
        {
            std::stringstream msg;
            msg << tid << " - " << ref[tid] << std::endl;
            std::cout << msg.str();
        }

        std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t0;
        if (print_stats)
        {
            std::stringstream ss;
            ss << "[" << tid << "] time elapsed for ref: " << duration.count() / 1000000.0 << "ms" << std::endl;
            std::cout << ss.str();
        }
    };

    t0 = std::chrono::steady_clock::now();
    tp->run(func_ref);
    duration = std::chrono::steady_clock::now() - t0;
    if (print_stats)
    {
        std::stringstream ss;
        ss << "[main] time elapsed for ref: " << duration.count() / 1000000.0 << "ms" << std::endl;
        std::cout << ss.str();
    }

    print_verbose("main -- 7");
    delete tp;
    print_verbose("main -- 8");

    bool ok = true;
    for (size_t tid = 0; tid < n_threads; tid++)
    {
        if (ref[tid] != res1[tid] || ref[tid] != res2[tid])
        {
            std::cout << "thread " << tid << " got different result: ref = " << ref[tid] << ", res1 = " << res1[tid]
                      << ", res2 = " << res2[tid] << std::endl;
            ok = false;
        }
    }

#ifndef SPU_JLUNA
    jl_atexit_hook(0);
#endif // !SPU_JLUNA

    // Jluna sandbox
    // ==============================================================================================
    // END
    return ok ? 0 : 1;
}
