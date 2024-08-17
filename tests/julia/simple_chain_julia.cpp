#include <atomic>
#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <jluna.hpp>
#include <memory>
#include <string>
#include <vector>

#include <streampu.hpp>
using namespace spu;
using namespace spu::runtime;

int
main(int argc, char** argv)
{
    jluna::initialize();

    // Julia sandbox ============================================================================================ BEGIN

    // jluna::Base["println"]("hello julia");

    // jluna::Main.safe_eval_file("../tests/julia/hello_world.jl");

    // jluna::Main.safe_eval("f(x) = x^x");
    // auto f = jluna::Main.safe_eval("return f");
    // int64_t result = f(2);
    // std::cout << "result = " << result << std::endl;

    // auto square = jluna::Main["square"];
    // int64_t result2 = square(3);
    // std::cout << "result2 = " << result2 << std::endl;

    // // jluna::Main.safe_eval("sqrt(-1)");

    // // declare lambda
    // auto myadd = [](int64_t a, int64_t b) -> int64_t
    // {
    //     return a + b;
    // };

    // // bind to Julia-side variable
    // jluna::Main.create_or_assign("myadd", jluna::as_julia_function<int64_t(int64_t, int64_t)>(myadd));

    // jluna::Main.safe_eval("println(\"myadd(1, 3): \", myadd(1, 3))");

    // jluna::Main.safe_eval_file("../tests/julia/myadd_call.jl");

    // // allocate C-array
    // // const int32_t c_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // int32_t* c_array = new int32_t[10];
    // std::iota(c_array, c_array +10, 2);

    // // create thin wrapper around it
    // auto* jl_array = jluna::unsafe::new_array_from_data(jluna::Int32_t, (void*) c_array, 10);

    // // wrapper can now be used Julia-side
    // {
    //     using namespace jluna;
    //     auto* println = jluna::unsafe::get_function(jluna::Base, "println"_sym);
    //     jluna::safe_call(println, jl_array);
    // }

    // delete[] c_array;

    // exit(0);

    // Julia sandbox ============================================================================================== END

    tools::Signal_handler::init();

    option longopts[] = { { "n-threads", required_argument, NULL, 't' },
                          { "n-inter-frames", required_argument, NULL, 'f' },
                          { "sleep-time", required_argument, NULL, 's' },
                          { "data-length", required_argument, NULL, 'd' },
                          { "n-exec", required_argument, NULL, 'e' },
                          { "dot-filepath", required_argument, NULL, 'o' },
                          { "copy-mode", no_argument, NULL, 'c' },
                          { "print-stats", no_argument, NULL, 'p' },
                          { "step-by-step", no_argument, NULL, 'b' },
                          { "debug", no_argument, NULL, 'g' },
                          { "verbose", no_argument, NULL, 'v' },
                          { "help", no_argument, NULL, 'h' },
                          { 0 } };

    size_t n_threads = std::thread::hardware_concurrency();
    size_t n_inter_frames = 1;
    size_t sleep_time_us = 5;
    size_t data_length = 2048;
    size_t n_exec = 100000;
    std::string dot_filepath;
    bool no_copy_mode = true;
    bool print_stats = false;
    bool step_by_step = false;
    bool debug = false;
    bool verbose = false;

    while (1)
    {
        const int opt = getopt_long(argc, argv, "t:f:s:d:e:o:cpbgvh", longopts, 0);
        if (opt == -1) break;
        switch (opt)
        {
            case 't':
                n_threads = atoi(optarg);
                break;
            case 'f':
                n_inter_frames = atoi(optarg);
                break;
            case 's':
                sleep_time_us = atoi(optarg);
                break;
            case 'd':
                data_length = atoi(optarg);
                break;
            case 'e':
                n_exec = atoi(optarg);
                break;
            case 'o':
                dot_filepath = std::string(optarg);
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
                std::cout << "  -t, --n-threads       "
                          << "Number of threads to run in parallel                                  "
                          << "[" << n_threads << "]" << std::endl;
                std::cout << "  -f, --n-inter-frames  "
                          << "Number of frames to process in one task                               "
                          << "[" << n_inter_frames << "]" << std::endl;
                std::cout << "  -s, --sleep-time      "
                          << "Sleep time duration in one task (microseconds)                        "
                          << "[" << sleep_time_us << "]" << std::endl;
                std::cout << "  -d, --data-length     "
                          << "Size of data to process in one task (in bytes)                        "
                          << "[" << data_length << "]" << std::endl;
                std::cout << "  -e, --n-exec          "
                          << "Number of sequence executions                                         "
                          << "[" << n_exec << "]" << std::endl;
                std::cout << "  -o, --dot-filepath    "
                          << "Path to dot output file                                               "
                          << "[" << (dot_filepath.empty() ? "empty" : "\"" + dot_filepath + "\"") << "]" << std::endl;
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

    std::cout << "#######################################" << std::endl;
    std::cout << "# Micro-benchmark: Simple chain Julia #" << std::endl;
    std::cout << "#######################################" << std::endl;
    std::cout << "#" << std::endl;

    std::cout << "# Command line arguments:" << std::endl;
    std::cout << "#   - n_threads      = " << n_threads << std::endl;
    std::cout << "#   - n_inter_frames = " << n_inter_frames << std::endl;
    std::cout << "#   - sleep_time_us  = " << sleep_time_us << std::endl;
    std::cout << "#   - data_length    = " << data_length << std::endl;
    std::cout << "#   - n_exec         = " << n_exec << std::endl;
    std::cout << "#   - dot_filepath   = " << (dot_filepath.empty() ? "[empty]" : dot_filepath.c_str()) << std::endl;
    std::cout << "#   - no_copy_mode   = " << (no_copy_mode ? "true" : "false") << std::endl;
    std::cout << "#   - print_stats    = " << (print_stats ? "true" : "false") << std::endl;
    std::cout << "#   - step_by_step   = " << (step_by_step ? "true" : "false") << std::endl;
    std::cout << "#   - debug          = " << (debug ? "true" : "false") << std::endl;
    std::cout << "#   - verbose        = " << (verbose ? "true" : "false") << std::endl;
    std::cout << "#" << std::endl;

    // modules creation
    module::Initializer<uint8_t> initializer(data_length);
    module::Finalizer<uint8_t> finalizer(data_length);

    module::Stateless_Julia incr;
    runtime::Task& t = incr.create_task("increment");
    incr.create_socket_in<uint8_t>(t, "in", data_length);
    incr.create_socket_out<uint8_t>(t, "out", data_length);
    incr.create_constant<uint32_t>(t, sleep_time_us * 1000); // wait time in ns
    // incr.create_constant<std::string>(t, "Adrien");
    incr.create_codelet_file(t, "../tests/julia/increment.jl");
    // incr.create_codelet(t, R""""(
    //     function increment(sck_in, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
    //         if cst_wait_time_ns > 0
    //             start_incr = time_ns()
    //         end

    //         for n in eachindex(sck_in)
    //             sck_out[n] = sck_in[n] + 1
    //         end

    //         if cst_wait_time_ns != 0
    //             stop_incr = time_ns()
    //             elapse_incr = stop_incr - start_incr

    //             while elapse_incr < cst_wait_time_ns
    //                 elapse_incr = time_ns() - start_incr
    //             end
    //         end

    //         return 0
    //     end
    // )"""");

    std::vector<std::shared_ptr<module::Stateless_Julia>> incs(6);
    for (size_t s = 0; s < incs.size(); s++)
    {
        incs[s].reset(incr.clone());
        incs[s]->set_custom_name("Inc" + std::to_string(s));
    }

    // sockets binding
    (*incs[0])["increment::in"] = initializer["initialize::out"];
    for (size_t s = 0; s < incs.size() - 1; s++)
        (*incs[s + 1])["increment::in"] = (*incs[s])["increment::out"];
    finalizer["finalize::in"] = (*incs[incs.size() - 1])["increment::out"];

    runtime::Sequence sequence_chain(initializer("initialize"), n_threads);
    sequence_chain.set_n_frames(n_inter_frames);
    sequence_chain.set_no_copy_mode(no_copy_mode);

    // warmup to compile Julia code
    auto t_start_warmup = std::chrono::steady_clock::now();
    sequence_chain.exec([]() { return true; });
    std::chrono::nanoseconds duration_warmup = std::chrono::steady_clock::now() - t_start_warmup;
    auto elapsed_time_warmup = duration_warmup.count() / 1000.f / 1000.f;
    std::cout << "# Julia warmup (= JIT compilation) time: " << elapsed_time_warmup << " ms" << std::endl;

    auto tid = 0;
    for (auto cur_initializer : sequence_chain.get_cloned_modules<module::Initializer<uint8_t>>(initializer))
    {
        std::vector<std::vector<uint8_t>> init_data(n_inter_frames, std::vector<uint8_t>(data_length, 0));
        for (size_t f = 0; f < n_inter_frames; f++)
            std::fill(init_data[f].begin(), init_data[f].end(), tid * n_inter_frames + f);
        cur_initializer->set_init_data(init_data);
        tid++;
    }

    if (!dot_filepath.empty())
    {
        std::ofstream file(dot_filepath);
        sequence_chain.export_dot(file);
    }

    // configuration of the sequence tasks
    for (auto& mod : sequence_chain.get_modules<module::Module>(false))
        for (auto& tsk : mod->tasks)
        {
            tsk->reset();
            tsk->set_debug(debug);       // disable the debug mode
            tsk->set_debug_limit(16);    // display only the 16 first bits if the debug mode is enabled
            tsk->set_stats(print_stats); // enable the statistics
            tsk->set_fast(true);         // enable the fast mode (= disable the useless verifs in the tasks)
        }

    if (verbose)
    {
        std::cout << std::endl;
        std::cout << "Helper information:" << std::endl;
        std::cout << "-------------------" << std::endl;
        tools::help(initializer);
        for (size_t s = 0; s < incs.size(); s++)
            tools::help(*incs[s]);
        tools::help(finalizer);
    }

    std::atomic<unsigned int> counter(0);
    auto t_start = std::chrono::steady_clock::now();
    if (!step_by_step)
    {
        // execute the sequence (multi-threaded)
        sequence_chain.exec([&counter, n_exec]() { return ++counter >= n_exec; });
    }
    else
    {
        do
            for (size_t tid = 0; tid < n_threads; tid++)
                while (sequence_chain.exec_step(tid))
                    ;
        while (++counter < (n_exec / n_threads));
    }

    std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

    auto elapsed_time = duration.count() / 1000.f / 1000.f;
    std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;

    size_t chain_sleep_time = 0;
    for (size_t i = 0; i < incs.size(); i++)
        chain_sleep_time += sleep_time_us * 1000;

    auto theoretical_time = (chain_sleep_time * n_exec * n_inter_frames) / 1000.f / 1000.f / n_threads;
    std::cout << "Sequence theoretical time: " << theoretical_time << " ms" << std::endl;

    // verification of the sequence execution
    bool tests_passed = true;
    tid = 0;
    for (auto cur_finalizer : sequence_chain.get_cloned_modules<module::Finalizer<uint8_t>>(finalizer))
    {
        for (size_t f = 0; f < n_inter_frames; f++)
        {
            const auto& final_data = cur_finalizer->get_final_data()[f];
            for (size_t d = 0; d < final_data.size(); d++)
            {
                auto expected = (int)(incs.size() + (tid * n_inter_frames + f));
                expected = expected % 256;
                if (final_data[d] != expected)
                {
                    std::cout << "# expected = " << +expected << " - obtained = " << +final_data[d] << " (d = " << d
                              << ", tid = " << tid << ")" << std::endl;
                    tests_passed = false;
                }
            }
        }
        tid++;
    }

    if (tests_passed)
        std::cout << "# " << rang::style::bold << rang::fg::green << "Tests passed!" << rang::style::reset << std::endl;
    else
        std::cout << "# " << rang::style::bold << rang::fg::red << "Tests failed :-(" << rang::style::reset
                  << std::endl;

    unsigned int test_results = !tests_passed;

    // display the statistics of the tasks (if enabled)
    if (print_stats)
    {
        std::cout << "#" << std::endl;
        tools::Stats::show(sequence_chain.get_modules_per_types(), true, false);
    }

    // sockets unbinding
    sequence_chain.set_n_frames(1);

    (*incs[0])["increment::in"].unbind(initializer["initialize::out"]);
    for (size_t s = 0; s < incs.size() - 1; s++)
        (*incs[s + 1])["increment::in"].unbind((*incs[s])["increment::out"]);
    finalizer["finalize::in"].unbind((*incs[incs.size() - 1])["increment::out"]);

    return test_results;
}
