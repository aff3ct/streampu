#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <streampu.hpp>
using namespace spu;
using namespace spu::runtime;

int
main(int argc, char** argv)
{
    tools::Signal_handler::init();

    option longopts[] = { { "n-threads", required_argument, NULL, 't' },
                          { "n-inter-frames", required_argument, NULL, 'f' },
                          { "sleep-time", required_argument, NULL, 's' },
                          { "data-length", required_argument, NULL, 'd' },
                          { "buffer-size", required_argument, NULL, 'u' },
                          { "dot-filepath", required_argument, NULL, 'o' },
                          { "copy-mode", no_argument, NULL, 'c' },
                          { "print-stats", no_argument, NULL, 'p' },
                          { "debug", no_argument, NULL, 'g' },
                          { "active-waiting", no_argument, NULL, 'w' },
                          { "help", no_argument, NULL, 'h' },
                          { 0 } };

    size_t n_threads = std::thread::hardware_concurrency();
    size_t n_inter_frames = 1;
    size_t sleep_time_us = 5;
    size_t data_length = 2048;
    size_t buffer_size = 16;
    std::string dot_filepath;
    bool print_stats = false;
    bool debug = false;
    bool active_waiting = false;

    while (1)
    {
        const int opt = getopt_long(argc, argv, "t:f:s:d:u:o:i:j:cpbgqwh", longopts, 0);
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
            case 'u':
                buffer_size = atoi(optarg);
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
                std::cout << "  -u, --buffer-size     "
                          << "Size of the buffer between the different stages of the pipeline       "
                          << "[" << data_length << "]" << std::endl;
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

    std::cout << "############################################################" << std::endl;
    std::cout << "# Micro-benchmark: Complexe pipeline increment and compare #" << std::endl;
    std::cout << "############################################################" << std::endl;
    std::cout << "#" << std::endl;

    std::cout << "# Command line arguments:" << std::endl;
    std::cout << "#   - n_threads      = " << n_threads << std::endl;
    std::cout << "#   - n_inter_frames = " << n_inter_frames << std::endl;
    std::cout << "#   - sleep_time_us  = " << sleep_time_us << std::endl;
    std::cout << "#   - data_length    = " << data_length << std::endl;
    std::cout << "#   - buffer_size    = " << buffer_size << std::endl;
    std::cout << "#   - dot_filepath   = " << (dot_filepath.empty() ? "[empty]" : dot_filepath.c_str()) << std::endl;
    std::cout << "#   - print_stats    = " << (print_stats ? "true" : "false") << std::endl;
    std::cout << "#   - debug          = " << (debug ? "true" : "false") << std::endl;
    std::cout << "#   - active_waiting = " << (active_waiting ? "true" : "false") << std::endl;
    std::cout << "#" << std::endl;

    module::Initializer<uint8_t> initializer(data_length);
    module::Finalizer<uint8_t> finalizer(data_length);

    std::vector<std::shared_ptr<module::Incrementer<uint8_t>>> incs(6);
    for (size_t s = 0; s < incs.size(); s++)
    {
        incs[s].reset(new module::Incrementer<uint8_t>(data_length));
        incs[s]->set_ns(sleep_time_us * 1000);
        incs[s]->set_custom_name("Inc" + std::to_string(s));
    }

    std::shared_ptr<module::Incrementer<uint8_t>> inc_calssique;
    inc_calssique.reset(new module::Incrementer<uint8_t>(data_length));
    inc_calssique->set_ns(sleep_time_us * 1000);
    inc_calssique->set_custom_name("Inc");

    // Stateless module used only for the test
    module::Stateless multi_comp;
    multi_comp.set_name("Multiplier_comparator");
    auto& task_multi_comp = multi_comp.create_task("multiply_compare");
    auto sock_0 = multi_comp.create_socket_fwd<uint8_t>(task_multi_comp, "fwd0", data_length);
    auto sock_1 = multi_comp.create_socket_fwd<uint8_t>(task_multi_comp, "fwd1", data_length);

    multi_comp.create_codelet(
      task_multi_comp,
      [sock_0, sock_1, data_length, incs](module::Module& m, runtime::Task& t, const size_t frame_id) -> int
      {
          auto tab_0 = t[sock_0].get_dataptr<uint8_t>();
          auto tab_1 = t[sock_1].get_dataptr<const uint8_t>();
          for (size_t i = 0; i < data_length; ++i)
              tab_0[i] = tab_0[i] * (incs.size() + 1) + 1;
          for (size_t i = 0; i < data_length; ++i)
              if (tab_0[i] != tab_1[i])
              {
                  std::cout << "Found different values => "
                            << " Tab_0 : " << unsigned(tab_0[i]) << ", Tab_1 : " << unsigned(tab_1[i]) << std::endl;
                  return runtime::status_t::FAILURE;
              }
          std::cout << "All the values are correct "
                    << "Expected : " << unsigned(tab_0[0]) << ", got : " << unsigned(tab_1[0]) << std::endl;
          return runtime::status_t::SUCCESS;
      });

    // sockets binding
    // clang-format off
    (*inc_calssique)[       "increment::in"  ] = initializer           [      "initialize::out" ];
    (*incs[0])      [      "incrementf::fwd" ] = (*inc_calssique)      [       "increment::out" ];
    // Bind the initial data to the multiplier
    multi_comp      ["multiply_compare::fwd0"] = initializer           [      "initialize::out" ];
    for (size_t s = 0; s < incs.size() -1; s++)
        (*incs[s+1])[      "incrementf::fwd" ] = (*incs[s])            [      "incrementf::fwd" ];
    multi_comp      ["multiply_compare::fwd1"] = (*incs[incs.size()-1])[      "incrementf::fwd" ];
    finalizer       [        "finalize::in"  ] = multi_comp            ["multiply_compare::fwd1"];
    // clang-format on

    std::unique_ptr<runtime::Pipeline> pipeline_chain;
    pipeline_chain.reset(
      new runtime::Pipeline(initializer[module::ini::tsk::initialize], // first task of the sequence
                            {
                              // pipeline stage 0, first & last tasks
                              { { &initializer("initialize") }, { &initializer("initialize") } },
                              // pipeline stage 1, first & last tasks
                              { { &(*inc_calssique)("increment") }, { &(*incs[incs.size() - 1])("incrementf") } },
                              // pipeline stage 2, first & last tasks
                              { { &multi_comp("multiply_compare") }, { &multi_comp("multiply_compare") } },
                              // pipeline stage 3, first task
                              { { &finalizer[module::fin::tsk::finalize] }, {} },
                            },
                            {
                              1,                         // number of threads in the stage 0
                              n_threads ? n_threads : 1, // number of threads in the stage 1
                              1,                         // number of threads in the stage 2
                              1                          // number of threads in the stage 3
                            },
                            {
                              buffer_size, // synchronization buffer size between stages 0 and 1
                              buffer_size, // synchronization buffer size between stages 1 and 2
                              buffer_size, // synchronization buffer size between stages 2 and 3
                            },
                            {
                              active_waiting, // type of waiting between stages 0 and 1 (true = active, false = passive)
                              active_waiting, // type of waiting between stages 1 and 2 (true = active, false = passive)
                              active_waiting, // type of waiting between stages 2 and 3 (true = active, false = passive)
                            }));
    pipeline_chain->set_n_frames(n_inter_frames);

    // Getting the input data
    auto tid = 0;
    for (auto cur_initializer :
         pipeline_chain.get()->get_stages()[0]->get_cloned_modules<module::Initializer<uint8_t>>(initializer))
    {
        std::vector<std::vector<uint8_t>> init_data(n_inter_frames, std::vector<uint8_t>(data_length, 0));
        for (size_t f = 0; f < n_inter_frames; f++)
            std::fill(init_data[f].begin(), init_data[f].end(), 1);
        cur_initializer->set_init_data(init_data);
        tid++;
    }

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
    pipeline_chain->exec([]() { return true; });
    std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;
    auto elapsed_time = duration.count() / 1000.f / 1000.f;
    std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;

    // verification of the sequence execution
    bool tests_passed = true;
    tid = 0;

    for (auto cur_finalizer : pipeline_chain.get()
                                ->get_stages()[pipeline_chain.get()->get_stages().size() - 1]
                                ->get_cloned_modules<module::Finalizer<uint8_t>>(finalizer))
    {
        for (size_t f = 0; f < n_inter_frames; f++)
        {
            const auto& final_data = cur_finalizer->get_final_data()[f];
            for (size_t d = 0; d < final_data.size(); d++)
            {
                auto expected = (int)(incs.size() + 2);
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

    // Sockets unbinding
    pipeline_chain->set_n_frames(1);
    pipeline_chain->unbind_adaptors();

    (*inc_calssique)[module::inc::sck::increment::in].unbind(initializer[module::ini::sck::initialize::out]);
    multi_comp["multiply_compare::fwd0"].unbind(initializer[module::ini::sck::initialize::out]);
    (*incs[0])[module::inc::sck::incrementf::fwd].unbind((*inc_calssique)[module::inc::sck::increment::out]);
    for (size_t s = 0; s < incs.size() - 1; s++)
        (*incs[s + 1])[module::inc::sck::incrementf::fwd].unbind((*incs[s])[module::inc::sck::incrementf::fwd]);
    multi_comp["multiply_compare::fwd1"].unbind((*incs[incs.size() - 1])[module::inc::sck::incrementf::fwd]);
    finalizer[module::fin::sck::finalize::in].unbind(multi_comp["multiply_compare::fwd1"]);

    return test_results;
}
