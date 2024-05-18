#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <hwloc.h>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <string>
#include <vector>

#include <streampu.hpp>
using namespace spu;
using namespace spu::runtime;

int
main(int argc, char** argv)
{
    tools::Signal_handler::init();

    option longopts[] = { { "n-inter-frames", required_argument, NULL, 'f' },
                          { "n-exec", required_argument, NULL, 'e' },
                          { "dot-filepath", required_argument, NULL, 'o' },
                          { "print-stats", no_argument, NULL, 'p' },
                          { "debug", no_argument, NULL, 'g' },
                          { "active-waiting", no_argument, NULL, 'w' },
                          { "help", no_argument, NULL, 'h' },
                          { 0 } };

    size_t n_inter_frames = 1;
    size_t data_length = 1;
    size_t n_exec = 100000;
    size_t buffer_size = 4;
    std::string dot_filepath;
    bool print_stats = false;
    bool debug = false;
    bool active_waiting = false;

    while (1)
    {
        const int opt = getopt_long(argc, argv, "f:e:o:pgwh", longopts, 0);
        if (opt == -1) break;
        switch (opt)
        {
            case 'f':
                n_inter_frames = atoi(optarg);
                break;
            case 'e':
                n_exec = atoi(optarg);
                break;
            case 'o':
                dot_filepath = std::string(optarg);
                break;
            case 'p':
                print_stats = true;
                break;
            case 'g':
                debug = true;
                break;
            case 'w':
                active_waiting = true;
                break;
            case 'h':
                std::cout << "usage: " << argv[0] << " [options]" << std::endl;
                std::cout << std::endl;
                std::cout << "  -f, --n-inter-frames  "
                          << "Number of frames to process in one task                               "
                          << "[" << n_inter_frames << "]" << std::endl;
                std::cout << "  -e, --n-exec          "
                          << "Number of sequence executions                                         "
                          << "[" << n_exec << "]" << std::endl;
                std::cout << "  -o, --dot-filepath    "
                          << "Path to dot output file                                               "
                          << "[" << (dot_filepath.empty() ? "empty" : "\"" + dot_filepath + "\"") << "]" << std::endl;
                std::cout << "  -p, --print-stats     "
                          << "Enable to print per task statistics (performance will be reduced)     "
                          << "[" << (print_stats ? "true" : "false") << "]" << std::endl;
                std::cout << "  -g, --debug           "
                          << "Enable task debug mode (print socket data)                            "
                          << "[" << (debug ? "true" : "false") << "]" << std::endl;
                std::cout << "  -w, --active-waiting  "
                          << "Enable active waiting in the pipeline synchronizations                "
                          << "[" << (active_waiting ? "true" : "false") << "]" << std::endl;
                std::cout << "  -h, --help            "
                          << "This help                                                             "
                          << "[false]" << std::endl;
                exit(0);
                break;
            default:
                break;
        }
    }

    std::cout << "###################################" << std::endl;
    std::cout << "# Micro-benchmark: Thread Pinning #" << std::endl;
    std::cout << "###################################" << std::endl;
    std::cout << "#" << std::endl;

    std::cout << "# Command line arguments:" << std::endl;
    std::cout << "#   - n_inter_frames = " << n_inter_frames << std::endl;
    std::cout << "#   - data_length    = " << data_length << std::endl;
    std::cout << "#   - n_exec         = " << n_exec << std::endl;
    std::cout << "#   - buffer_size    = " << buffer_size << std::endl;
    std::cout << "#   - dot_filepath   = " << (dot_filepath.empty() ? "[empty]" : dot_filepath.c_str()) << std::endl;
    std::cout << "#   - print_stats    = " << (print_stats ? "true" : "false") << std::endl;
    std::cout << "#   - debug          = " << (debug ? "true" : "false") << std::endl;
    std::cout << "#   - active_waiting = " << (active_waiting ? "true" : "false") << std::endl;
    std::cout << "#" << std::endl;

    tools::Thread_pinning::init();

    std::vector<std::shared_ptr<module::Finalizer<uint32_t>>> finalizers(3);
    for (size_t s = 0; s < finalizers.size(); s++)
        finalizers[s].reset(new module::Finalizer<uint32_t>(data_length, n_exec));

    // Getting hwloc topology
    hwloc_topology_t g_topology;
    hwloc_topology_init(&g_topology);
    hwloc_topology_load(g_topology);

    // Stateless module, with a task which forwards the current hardware core of the running thread
    module::Stateless pin_mod;
    pin_mod.set_name("Pinner");
    auto& pin_task = pin_mod.create_task("pin");
    auto sck_val = pin_mod.create_socket_out<uint32_t>(pin_task, "val", data_length);

    pin_mod.create_codelet(pin_task,
                           [sck_val, g_topology](module::Module& m, runtime::Task& t, const size_t frame_id) -> int
                           {
                               auto core = sched_getcpu();
                               auto pu_obj = hwloc_get_pu_obj_by_os_index(g_topology, core);
                               *t[sck_val].get_dataptr<uint32_t>() = pu_obj->logical_index;

                               return runtime::status_t::SUCCESS;
                           });

    module::Stateless* pin_mod1 = &pin_mod;
    module::Stateless* pin_mod2 = pin_mod.clone();
    module::Stateless* pin_mod3 = pin_mod.clone();

    // sockets binding
    (*pin_mod2)("pin") = (*pin_mod1)["pin::val"];
    (*pin_mod3)("pin") = (*pin_mod2)["pin::val"];
    (*finalizers[0])["finalize::in"] = (*pin_mod1)["pin::val"];
    (*finalizers[1])["finalize::in"] = (*pin_mod2)["pin::val"];
    (*finalizers[2])["finalize::in"] = (*pin_mod3)["pin::val"];

    std::unique_ptr<runtime::Pipeline> pipeline_chain;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////// PIPELINE EXEC //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    pipeline_chain.reset(new runtime::Pipeline(
      (*pin_mod1)("pin"), // first task of the sequence
      {
        // pipeline stage 0
        { { &(*pin_mod1)("pin") }, { &(*pin_mod1)("pin") } },
        // pipeline stage 1
        { { &(*pin_mod2)("pin") }, { &(*pin_mod2)("pin") } },
        // pipeline stage 2
        { { &(*pin_mod3)("pin"), &((*finalizers[0])("finalize")), &((*finalizers[1])("finalize")) }, {} },
      },
      {
        1, // number of threads in the stage 0
        3, // number of threads in the stage 1
        1, // number of threads in the stage 2
      },
      {
        buffer_size, // synchronization buffer size between stages 0 and 1
        buffer_size, // synchronization buffer size between stages 1 and 2
      },
      {
        active_waiting, // type of waiting between stages 0 and 1 (true = active, false = passive)
        active_waiting, // type of waiting between stages 1 and 2 (true = active, false = passive)
      },
      {
        true, // Pinning activation for stage 0
        true, // Pinning activation for stage 1
        true, // Pinning activation for stage 2
      },
      " PU_0 | PU_0; PU_1; PU_2 | PU_3") // explicit thread pinning
    );
    pipeline_chain->set_n_frames(n_inter_frames);

    if (!dot_filepath.empty())
    {
        std::ofstream file(dot_filepath);
        pipeline_chain->export_dot(file);
    }

    // configuration of the sequence tasks
    for (auto& mod : pipeline_chain->get_modules<module::Module>(false))
        for (auto& tsk : mod->tasks)
        {
            tsk->reset();
            tsk->set_debug(debug);       // disable the debug mode
            tsk->set_debug_limit(16);    // display only the 16 first bits if the debug mode is enabled
            tsk->set_stats(print_stats); // enable the statistics
            tsk->set_fast(true);         // enable the fast mode (= disable the useless verifs in the tasks)
        }

    auto t_start = std::chrono::steady_clock::now();
    std::atomic<unsigned int> counter(0);
    pipeline_chain->exec([&counter, n_exec]() { return ++counter >= n_exec; });
    std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;
    auto elapsed_time = duration.count() / 1000.f / 1000.f;
    std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;

    // verification of the pipeline thread binding
    bool tests_passed = true;
    for (size_t i = 0; i < n_exec; i++)
    {
        for (size_t f = 0; f < n_inter_frames; f++)
        {
            if (finalizers[0]->get_histo_data()[i][f][0] != 0)
            {
                tests_passed = false;
                std::cout << "# Thread is not pin to expected value for the stream n°" << i << " and fra n°" << f
                          << std::endl
                          << "Stage: " << 0 << ", Thread number: " << 0 << std::endl
                          << "Expected: 0, Real: " << finalizers[0]->get_histo_data()[i][f][0] << std::endl;
#ifndef SPU_HWLOC
                std::cout << "You need to compile with the '-DSPU_LINK_HWLOC=ON' CMake option!" << std::endl;
#endif
                break;
            }

            if (finalizers[1]->get_histo_data()[i][f][0] != (i % 3))
            {
                tests_passed = false;
                std::cout << "# Thread is not pin to expected value for the stream n°" << i << " and fra n°" << f
                          << std::endl
                          << "Stage: " << 1 << ", Thread number: " << (i % 3) << std::endl
                          << "Expected: " << (i % 3) << ", Real: " << finalizers[1]->get_histo_data()[i][f][0]
                          << std::endl;
#ifndef SPU_HWLOC
                std::cout << "You need to compile with the '-DSPU_LINK_HWLOC=ON' CMake option!" << std::endl;
#endif
                break;
            }

            if (finalizers[2]->get_histo_data()[i][f][0] != 3)
            {
                tests_passed = false;
                std::cout << "# Thread is not pin to expected value for the stream n°" << i << " and fra n°" << f
                          << std::endl
                          << "Stage: " << 2 << ", Thread number: " << 0 << std::endl
                          << "Expected: 3, Real: " << finalizers[2]->get_histo_data()[i][f][0] << std::endl;
#ifndef SPU_HWLOC
                std::cout << "You need to compile with the '-DSPU_LINK_HWLOC=ON' CMake option!" << std::endl;
#endif
                break;
            }
        }
        if (!tests_passed) break;
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
        auto stages = pipeline_chain->get_stages();
        for (size_t s = 0; s < stages.size(); s++)
        {
            const int n_threads = stages[s]->get_n_threads();
            std::cout << "#" << std::endl
                      << "# Pipeline stage " << s << " (" << n_threads << " thread(s)): " << std::endl;
            tools::Stats::show(stages[s]->get_tasks_per_types(), true, false);
        }
    }

    pipeline_chain->set_n_frames(1);
    pipeline_chain->unbind_adaptors();

    (*pin_mod2)("pin").unbind((*pin_mod1)["pin::val"]);
    (*pin_mod3)("pin").unbind((*pin_mod2)["pin::val"]);
    (*finalizers[0])["finalize::in"].unbind((*pin_mod1)["pin::val"]);
    (*finalizers[1])["finalize::in"].unbind((*pin_mod2)["pin::val"]);
    (*finalizers[2])["finalize::in"].unbind((*pin_mod3)["pin::val"]);

    return test_results;
}
