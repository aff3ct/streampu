#include <cstddef>
#include <getopt.h>
#include <iostream>
#include <julia.h>
#include <pthread.h>
#include <sched.h>
#include <sstream>
#include <vector>

struct _args
{
    pthread_mutex_t lock;
    pthread_barrier_t barrier;
    size_t n_threads;
    size_t cpt;
    size_t N;
};

void
init_args(struct _args* args, size_t n_threads)
{
    pthread_mutex_init(&args->lock, NULL);
    pthread_barrier_init(&args->barrier, NULL, n_threads + 1);
    args->n_threads = n_threads;
    args->cpt = 0;
    args->N = 1ul << 20;
}

void*
thread_code(void* args)
{
    struct _args* data = (struct _args*)args;
    auto tid = pthread_self();
    jl_gcframe_t** gc_struct;

    pthread_mutex_lock(&data->lock);
    gc_struct = jl_adopt_thread();
    pthread_mutex_unlock(&data->lock);
    jl_task_t* ct = jl_current_task;
    int8_t gc_state = jl_gc_safe_enter(ct->ptls);

    {
        std::stringstream msg;
        msg << '[' << tid << ": " << (void*)gc_struct << std::endl;
        std::cout << msg.str();
    }

    pthread_barrier_wait(&data->barrier);

    jl_value_t* v = NULL;
    for (size_t i = 0; i < data->N; i++)
    {
        v = jl_box_uint64((u_int64_t)i);
        // jl_gc_collect(JL_GC_FULL);
        // std::cout << tid << ": gc " << i << std::endl;
    }
    return NULL;
}

int
main(int argc, char** argv)
{
    jl_init();
    jl_value_t* str = jl_cstr_to_string("Hello World!");
    size_t n_threads = 2;
    size_t N = 1 << 20;
    bool verbose = false;

    option longopts[] = { { "threads", required_argument, NULL, 't' }, { "n-iter", required_argument, NULL, 'n' },
                          { "debug", no_argument, NULL, 'g' },         { "verbose", no_argument, NULL, 'v' },
                          { "help", no_argument, NULL, 'h' },          { 0 } };
    while (1)
    {
        const int opt = getopt_long(argc, argv, "t:f:j:pbgvh", longopts, 0);
        if (opt == -1) break;
        switch (opt)
        {
            case 't':
                n_threads = atoi(optarg);
                break;
            case 'n':
                N = atoi(optarg);
                break;
            case 'g':
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
                std::cout << "  -n, --n-iter          "
                          << "Number of iteration in thrads code                                    "
                          << "[" << N << "]" << std::endl;
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

    struct _args args;
    init_args(&args, n_threads);
    args.N = N;

    jl_gc_enable(0);
    if (verbose) std::cout << "gc: " << jl_gc_is_enabled() << std::endl;

    std::vector<pthread_t> threads(n_threads);
    for (size_t i = 0; i < n_threads; i++)
    {
        pthread_create(&threads[i], NULL, thread_code, (void*)&args);
    }

    jl_task_t* ct = jl_current_task;
    int8_t gc_state = jl_gc_safe_enter(ct->ptls);

    if (verbose)
    {
        std::stringstream msg;
        msg << "[main]: " << jl_gc_is_enabled() << std::endl;
        std::cout << msg.str();
    }

    pthread_barrier_wait(&args.barrier);
    jl_gc_enable(1);

    if (verbose)
    {
        std::stringstream msg;
        msg << "[main]: " << jl_gc_is_enabled() << std::endl;
        std::cout << msg.str();
    }

    for (auto& t : threads)
    {
        void* ret;
        pthread_join(t, &ret);
        if (verbose) std::cout << "ret: " << ret << std::endl;
    }
    jl_atexit_hook(0);
    return 0;
}
