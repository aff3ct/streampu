#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <julia.h>
#include <memory>
#include <string>
#include <vector>

#include <streampu.hpp>
using namespace spu;
using namespace spu::runtime;

std::ifstream::pos_type
filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

bool
compare_files(const std::string& filename1, const std::string& filename2)
{
    std::ifstream file1(filename1, std::ifstream::ate | std::ifstream::binary); // open file at the end
    std::ifstream file2(filename2, std::ifstream::ate | std::ifstream::binary); // open file at the end
    const std::ifstream::pos_type fileSize = file1.tellg();

    if (fileSize != file2.tellg()) return false; // different file size

    file1.seekg(0); // rewind
    file2.seekg(0); // rewind

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);

    return std::equal(begin1, std::istreambuf_iterator<char>(), begin2); // Second argument is end-of-range iterator
}

int
main(int argc, char** argv)
{
#ifdef SPU_JLUNA
    jluna::initialize(1);
#else
    jl_init();
#endif
    tools::Julia::init(256);

    tools::Signal_handler::init();

    option longopts[] = { { "n-threads", required_argument, NULL, 't' },
                          { "n-inter-frames", required_argument, NULL, 'f' },
                          { "sleep-time", required_argument, NULL, 's' },
                          { "data-length", required_argument, NULL, 'd' },
                          { "n-exec", required_argument, NULL, 'e' },
                          { "buffer-size", required_argument, NULL, 'u' },
                          { "dot-filepath", required_argument, NULL, 'o' },
                          { "in-filepath", required_argument, NULL, 'i' },
                          { "out-filepath", required_argument, NULL, 'j' },
                          { "copy-mode", no_argument, NULL, 'c' },
                          { "print-stats", no_argument, NULL, 'p' },
                          { "step-by-step", no_argument, NULL, 'b' },
                          { "debug", no_argument, NULL, 'g' },
                          { "force-sequence", no_argument, NULL, 'q' },
                          { "active-waiting", no_argument, NULL, 'w' },
                          { "task-autoalloc", no_argument, NULL, 'k' },
                          { "jl-safe", no_argument, NULL, 'J' },
                          { "jl-extern", no_argument, NULL, 'E' },
                          { "hybrid", no_argument, NULL, 'H' },
                          { "gc-stress", no_argument, NULL, 'G' },
                          { "help", no_argument, NULL, 'h' },
                          { NULL, 0, NULL, 0 } };

    size_t n_threads = std::thread::hardware_concurrency();
    size_t n_inter_frames = 1;
    size_t sleep_time_us = 5;
    size_t data_length = 2048;
    size_t n_exec = 100;
    size_t buffer_size = 16;
    std::string dot_filepath;
    std::string in_filepath;
    std::string out_filepath = "file_julia.out";
    bool no_copy_mode = true;
    bool print_stats = false;
    bool step_by_step = false;
    bool debug = false;
    bool force_sequence = false;
    bool active_waiting = false;
    bool task_autoalloc = false;
    bool jl_safe = false;
    bool jl_extern = false;
    bool hybrid = false;
    bool gc_stress = false;

    while (1)
    {
        const int opt = getopt_long(argc, argv, "t:f:s:d:e:u:o:i:j:cpbgqwkJEHGh", longopts, 0);
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
            case 'u':
                buffer_size = atoi(optarg);
                break;
            case 'o':
                dot_filepath = std::string(optarg);
                break;
            case 'i':
                in_filepath = std::string(optarg);
                break;
            case 'j':
                out_filepath = std::string(optarg);
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
            case 'w':
                active_waiting = true;
                break;
            case 'q':
                force_sequence = true;
                break;
            case 'k':
                task_autoalloc = true;
                break;
            case 'J':
                jl_safe = true;
                break;
            case 'E':
                jl_extern = true;
                break;
            case 'H':
                hybrid = true;
                break;
            case 'G':
                gc_stress = true;
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
                          << "Number of sequence executions (when no input file is given)           "
                          << "[" << n_exec << "]" << std::endl;
                std::cout << "  -u, --buffer-size     "
                          << "Size of the buffer between the different stages of the pipeline       "
                          << "[" << buffer_size << "]" << std::endl;
                std::cout << "  -o, --dot-filepath    "
                          << "Path to dot output file                                               "
                          << "[" << (dot_filepath.empty() ? "empty" : "\"" + dot_filepath + "\"") << "]" << std::endl;
                std::cout << "  -i, --in-filepath     "
                          << "Path to the input file (used to generate bits of the chain)           "
                          << "[" << (in_filepath.empty() ? "empty" : "\"" + in_filepath + "\"") << "]" << std::endl;
                std::cout << "  -j, --out-filepath    "
                          << "Path to the output file (written at the end of the chain)             "
                          << "[" << (out_filepath.empty() ? "empty" : "\"" + out_filepath + "\"") << "]" << std::endl;
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
                std::cout << "  -q, --force-sequence  "
                          << "Force sequence instead of pipeline                                    "
                          << "[" << (force_sequence ? "true" : "false") << "]" << std::endl;
                std::cout << "  -w, --active-waiting  "
                          << "Enable active waiting in the pipeline synchronizations                "
                          << "[" << (active_waiting ? "true" : "false") << "]" << std::endl;
                std::cout << "  -k, --task-autoalloc  "
                          << "Enable task SOUT autoalloc mode                                       "
                          << "[" << (task_autoalloc ? "true" : "false") << "]" << std::endl;
                std::cout << "  -J, --jl-safe         "
                          << "Enable Julia safe mode (throw exceptions to C++)                      "
                          << "[" << (jl_safe ? "true" : "false") << "]" << std::endl;
                std::cout << "  -E, --jl-extern       "
                          << "Evaluate the Julia code from an external file                         "
                          << "[" << (jl_extern ? "true" : "false") << "]" << std::endl;
                std::cout << "  -H, --hybrid          "
                          << "Mix C++ Incrementer and Julia Stateless_Julia stages                  "
                          << "[" << (hybrid ? "true" : "false") << "]" << std::endl;
                std::cout << "  -G, --gc-stress       "
                          << "Allocate temporary Julia arrays to stress-test multi-stage GC         "
                          << "[" << (gc_stress ? "true" : "false") << "]" << std::endl;
                std::cout << "  -h, --help            "
                          << "This help                                                             "
                          << "[false]" << std::endl;
                exit(0);
                break;
            default:
                break;
        }
    }

    std::cout << "##########################################" << std::endl;
    std::cout << "# Micro-benchmark: Simple pipeline Julia #" << std::endl;
    std::cout << "##########################################" << std::endl;
    std::cout << "#" << std::endl;

    std::cout << "# Command line arguments:" << std::endl;
    std::cout << "#   - n_threads      = " << n_threads << std::endl;
    std::cout << "#   - n_inter_frames = " << n_inter_frames << std::endl;
    std::cout << "#   - sleep_time_us  = " << sleep_time_us << std::endl;
    std::cout << "#   - data_length    = " << data_length << std::endl;
    std::cout << "#   - n_exec         = " << n_exec << std::endl;
    std::cout << "#   - buffer_size    = " << buffer_size << std::endl;
    std::cout << "#   - dot_filepath   = " << (dot_filepath.empty() ? "[empty]" : dot_filepath.c_str()) << std::endl;
    std::cout << "#   - in_filepath    = " << (in_filepath.empty() ? "[empty]" : in_filepath.c_str()) << std::endl;
    std::cout << "#   - out_filepath   = " << (out_filepath.empty() ? "[empty]" : out_filepath.c_str()) << std::endl;
    std::cout << "#   - no_copy_mode   = " << (no_copy_mode ? "true" : "false") << std::endl;
    std::cout << "#   - print_stats    = " << (print_stats ? "true" : "false") << std::endl;
    std::cout << "#   - step_by_step   = " << (step_by_step ? "true" : "false") << std::endl;
    std::cout << "#   - debug          = " << (debug ? "true" : "false") << std::endl;
    std::cout << "#   - force_sequence = " << (force_sequence ? "true" : "false") << std::endl;
    std::cout << "#   - active_waiting = " << (active_waiting ? "true" : "false") << std::endl;
    std::cout << "#   - task_autoalloc = " << (task_autoalloc ? "true" : "false") << std::endl;
    std::cout << "#   - jl_safe        = " << (jl_safe ? "true" : "false") << std::endl;
    std::cout << "#   - jl_extern      = " << (jl_extern ? "true" : "false") << std::endl;
    std::cout << "#   - hybrid         = " << (hybrid ? "true" : "false") << std::endl;
    std::cout << "#   - gc_stress      = " << (gc_stress ? "true" : "false") << std::endl;
    std::cout << "#" << std::endl;

    tools::Buffer_allocator::set_task_autoalloc(task_autoalloc);

    if (!force_sequence && !no_copy_mode)
        std::clog << rang::tag::warning << "'no_copy_mode' has no effect with pipeline (it is always enable)"
                  << std::endl;
    if (!force_sequence && step_by_step)
        std::clog << rang::tag::warning << "'step_by_step' is not available with pipeline" << std::endl;

    // ---------------------------------------------------------------------------------------------------------
    // Mode 1: File I/O Pipeline (Source_user_binary -> 6x Stateless_Julia relay -> Sink_user_binary)
    // ---------------------------------------------------------------------------------------------------------
    if (!in_filepath.empty())
    {
        if (force_sequence && n_threads > 1)
            std::clog << rang::tag::warning
                      << "Sequence mode only supports a single thread (User-Source/Sinks are not clonable)"
                      << std::endl;

        const bool auto_reset = false;
        module::Source_user_binary<uint8_t> source(data_length, in_filepath, auto_reset);
        module::Sink_user_binary<uint8_t> sink(data_length, out_filepath);

        module::Stateless_Julia rly_proto;
        runtime::Task& t = rly_proto.create_task("relay");
        rly_proto.create_socket_in<uint8_t>(t, "in", data_length);
        rly_proto.create_socket_out<uint8_t>(t, "out", data_length);
        rly_proto.create_constant<uint32_t>(t, sleep_time_us * 1000);
        if (jl_extern)
            rly_proto.create_codelet_file(t, "../tests/julia/incr_relay.jl");
        else if (gc_stress)
            rly_proto.create_codelet(t, R""""(
                function relay(sck_in, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
                    if cst_wait_time_ns > 0
                        start_rly = time_ns()
                    end

                    tmp = Vector{eltype(sck_in)}(undef, length(sck_in))
                    for n in eachindex(sck_in)
                        tmp[n] = sck_in[n]
                        sck_out[n] = tmp[n]
                    end

                    if cst_wait_time_ns != 0
                        stop_rly = time_ns()
                        elapse_rly = stop_rly - start_rly
                        while elapse_rly < cst_wait_time_ns
                            elapse_rly = time_ns() - start_rly
                        end
                    end

                    return 0
                end
            )"""");
        else
            rly_proto.create_codelet(t, R""""(
                function relay(sck_in, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
                    if cst_wait_time_ns > 0
                        start_rly = time_ns()
                    end

                    for n in eachindex(sck_in)
                        sck_out[n] = sck_in[n]
                    end

                    if cst_wait_time_ns != 0
                        stop_rly = time_ns()
                        elapse_rly = stop_rly - start_rly
                        while elapse_rly < cst_wait_time_ns
                            elapse_rly = time_ns() - start_rly
                        end
                    end

                    return 0
                end
            )"""");

        std::vector<std::shared_ptr<module::Stateless_Julia>> rlys(6);
        for (size_t s = 0; s < rlys.size(); s++)
        {
            rlys[s].reset(rly_proto.clone());
            rlys[s]->set_custom_name("Relayer_Jl" + std::to_string(s));
            rlys[s]->set_jl_safety(jl_safe);
        }

        // sockets binding
        (*rlys[0])["relay::in"] = source["generate::out_data"];
        for (size_t s = 0; s < rlys.size() - 1; s++)
            (*rlys[s + 1])["relay::in"] = (*rlys[s])["relay::out"];
        sink["send_count::in_data"] = (*rlys[rlys.size() - 1])["relay::out"];
        sink["send_count::in_count"] = source["generate::out_count"];

        std::unique_ptr<runtime::Sequence> sequence_chain;
        std::unique_ptr<runtime::Pipeline> pipeline_chain;
        if (force_sequence)
        {
            sequence_chain.reset(new runtime::Sequence(source("generate"), n_threads));
            sequence_chain->set_n_frames(n_inter_frames);
            sequence_chain->set_no_copy_mode(no_copy_mode);

            if (!dot_filepath.empty())
            {
                std::ofstream file(dot_filepath);
                sequence_chain->export_dot(file);
            }

            for (auto& mod : sequence_chain->get_modules<module::Module>(false))
                for (auto& tsk : mod->tasks)
                {
                    tsk->reset();
                    tsk->set_debug(debug);
                    tsk->set_debug_limit(16);
                    tsk->set_stats(print_stats);
                    tsk->set_fast(true);
                }

            auto t_start = std::chrono::steady_clock::now();
            if (!step_by_step)
                sequence_chain->exec();
            else
            {
                do
                {
                    try
                    {
                        for (size_t tid = 0; tid < n_threads; tid++)
                            while (sequence_chain->exec_step(tid))
                                ;
                    }
                    catch (tools::processing_aborted&)
                    { /* do nothing */
                    }
                } while (!source.is_done());
            }
            std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;
            auto elapsed_time = duration.count() / 1000.f / 1000.f;
            std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;
        }
        else
        {
            pipeline_chain.reset(new runtime::Pipeline(
              source("generate"),
              {
                // pipeline stage 0: first & last tasks
                { { &source("generate") }, { &source("generate") } },
                // pipeline stage 1: first & last tasks
                { { &(*rlys[0])("relay") }, { &(*rlys[rlys.size() - 1])("relay") } },
                // pipeline stage 2: first task
                { { &sink("send_count") }, {} },
              },
              {
                1,                         // number of threads in stage 0
                n_threads ? n_threads : 1, // number of threads in stage 1
                1                          // number of threads in stage 2
              },
              {
                buffer_size, // synchronization buffer size between stages 0 and 1
                buffer_size, // synchronization buffer size between stages 1 and 2
              },
              {
                active_waiting, // type of waiting between stages 0 and 1
                active_waiting, // type of waiting between stages 1 and 2
              }));
            pipeline_chain->set_n_frames(n_inter_frames);

            if (!dot_filepath.empty())
            {
                std::ofstream file(dot_filepath);
                pipeline_chain->export_dot(file);
            }

            for (auto& mod : pipeline_chain->get_modules<module::Module>(false))
                for (auto& tsk : mod->tasks)
                {
                    tsk->reset();
                    tsk->set_debug(debug);
                    tsk->set_debug_limit(16);
                    tsk->set_stats(print_stats);
                    tsk->set_fast(true);
                }

            auto t_start = std::chrono::steady_clock::now();
            pipeline_chain->exec();
            std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;
            auto elapsed_time = duration.count() / 1000.f / 1000.f;
            std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;
        }

        size_t in_filesize = filesize(in_filepath.c_str());
        size_t n_frames = ((int)std::ceil((float)(in_filesize * 8) / (float)(data_length * n_inter_frames)));
        auto theoretical_time =
          (n_frames * (rlys.size() * sleep_time_us * 1000) * n_inter_frames) / 1000.f / 1000.f / n_threads;
        std::cout << "Sequence theoretical time: " << theoretical_time << " ms" << std::endl;

        bool tests_passed = compare_files(in_filepath, out_filepath);
        if (tests_passed)
            std::cout << "# " << rang::style::bold << rang::fg::green << "Tests passed!" << rang::style::reset
                      << std::endl;
        else
            std::cout << "# " << rang::style::bold << rang::fg::red << "Tests failed :-(" << rang::style::reset
                      << std::endl;
        unsigned int test_results = !tests_passed;

        if (print_stats)
        {
            std::cout << "#" << std::endl;
            if (force_sequence)
                tools::Stats::show(sequence_chain->get_modules_per_types(), true, false);
            else
            {
                auto stages = pipeline_chain->get_stages();
                for (size_t s = 0; s < stages.size(); s++)
                {
                    const int st_threads = stages[s]->get_n_threads();
                    std::cout << "#" << std::endl
                              << "# Pipeline stage " << s << " (" << st_threads << " thread(s)): " << std::endl;
                    tools::Stats::show(stages[s]->get_tasks_per_types(), true, false);
                }
            }
        }

        if (force_sequence)
            sequence_chain->set_n_frames(1);
        else
        {
            pipeline_chain->set_n_frames(1);
            pipeline_chain->unbind_adaptors();
        }
        (*rlys[0])["relay::in"].unbind(source["generate::out_data"]);
        for (size_t s = 0; s < rlys.size() - 1; s++)
            (*rlys[s + 1])["relay::in"].unbind((*rlys[s])["relay::out"]);
        sink["send_count::in_data"].unbind((*rlys[rlys.size() - 1])["relay::out"]);
        sink["send_count::in_count"].unbind(source["generate::out_count"]);

        return test_results;
    }

    // ---------------------------------------------------------------------------------------------------------
    // Mode 2: Multi-stage Incrementer Pipeline (Initializer -> Stage 1 (3x Inc) -> Stage 2 (3x Inc) -> Finalizer)
    // ---------------------------------------------------------------------------------------------------------
    module::Initializer<uint8_t> initializer(data_length);
    module::Finalizer<uint8_t> finalizer(data_length);

    module::Stateless_Julia incr_proto;
    runtime::Task& t = incr_proto.create_task("increment");
    incr_proto.create_socket_in<uint8_t>(t, "in", data_length);
    incr_proto.create_socket_out<uint8_t>(t, "out", data_length);
    incr_proto.create_constant<uint32_t>(t, sleep_time_us * 1000);
    if (jl_extern)
        incr_proto.create_codelet_file(t, "../tests/julia/incr_relay.jl");
    else if (gc_stress)
        incr_proto.create_codelet(t, R""""(
            function increment(sck_in, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
                if cst_wait_time_ns > 0
                    start_incr = time_ns()
                end

                tmp = Vector{eltype(sck_in)}(undef, length(sck_in))
                for n in eachindex(sck_in)
                    tmp[n] = sck_in[n] + one(eltype(sck_in))
                    sck_out[n] = tmp[n]
                end

                if cst_wait_time_ns != 0
                    stop_incr = time_ns()
                    elapse_incr = stop_incr - start_incr
                    while elapse_incr < cst_wait_time_ns
                        elapse_incr = time_ns() - start_incr
                    end
                end

                return 0
            end
        )"""");
    else
        incr_proto.create_codelet(t, R""""(
            function increment(sck_in, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
                if cst_wait_time_ns > 0
                    start_incr = time_ns()
                end

                for n in eachindex(sck_in)
                    sck_out[n] = sck_in[n] + one(eltype(sck_in))
                end

                if cst_wait_time_ns != 0
                    stop_incr = time_ns()
                    elapse_incr = stop_incr - start_incr
                    while elapse_incr < cst_wait_time_ns
                        elapse_incr = time_ns() - start_incr
                    end
                end

                return 0
            end
        )"""");

    const size_t n_stage1 = 3;
    const size_t n_stage2 = 3;
    std::vector<std::shared_ptr<module::Stateless_Julia>> incs_jl1(n_stage1);
    for (size_t s = 0; s < n_stage1; s++)
    {
        incs_jl1[s].reset(incr_proto.clone());
        incs_jl1[s]->set_custom_name("Inc_Jl_S1_" + std::to_string(s));
        incs_jl1[s]->set_jl_safety(jl_safe);
    }

    std::vector<std::shared_ptr<module::Stateless_Julia>> incs_jl2(n_stage2);
    std::vector<std::shared_ptr<module::Incrementer<uint8_t>>> incs_cpp2(n_stage2);
    for (size_t s = 0; s < n_stage2; s++)
    {
        if (hybrid)
        {
            incs_cpp2[s].reset(new module::Incrementer<uint8_t>(data_length));
            incs_cpp2[s]->set_ns(sleep_time_us * 1000);
            incs_cpp2[s]->set_custom_name("Inc_Cpp_S2_" + std::to_string(s));
        }
        else
        {
            incs_jl2[s].reset(incr_proto.clone());
            incs_jl2[s]->set_custom_name("Inc_Jl_S2_" + std::to_string(s));
            incs_jl2[s]->set_jl_safety(jl_safe);
        }
    }

    // sockets binding
    (*incs_jl1[0])["increment::in"] = initializer["initialize::out"];
    for (size_t s = 0; s < n_stage1 - 1; s++)
        (*incs_jl1[s + 1])["increment::in"] = (*incs_jl1[s])["increment::out"];

    if (hybrid)
    {
        (*incs_cpp2[0])["increment::in"] = (*incs_jl1[n_stage1 - 1])["increment::out"];
        for (size_t s = 0; s < n_stage2 - 1; s++)
            (*incs_cpp2[s + 1])["increment::in"] = (*incs_cpp2[s])["increment::out"];
        finalizer["finalize::in"] = (*incs_cpp2[n_stage2 - 1])["increment::out"];
    }
    else
    {
        (*incs_jl2[0])["increment::in"] = (*incs_jl1[n_stage1 - 1])["increment::out"];
        for (size_t s = 0; s < n_stage2 - 1; s++)
            (*incs_jl2[s + 1])["increment::in"] = (*incs_jl2[s])["increment::out"];
        finalizer["finalize::in"] = (*incs_jl2[n_stage2 - 1])["increment::out"];
    }

    std::unique_ptr<runtime::Sequence> sequence_chain;
    std::unique_ptr<runtime::Pipeline> pipeline_chain;
    std::vector<module::Finalizer<uint8_t>*> finalizer_list;

    if (force_sequence)
    {
        sequence_chain.reset(new runtime::Sequence(initializer("initialize"), n_threads ? n_threads : 1));
        sequence_chain->set_n_frames(n_inter_frames);
        sequence_chain->set_no_copy_mode(no_copy_mode);

        size_t tid = 0;
        for (auto cur_initializer : sequence_chain->get_cloned_modules<module::Initializer<uint8_t>>(initializer))
        {
            std::vector<std::vector<uint8_t>> init_data(n_inter_frames, std::vector<uint8_t>(data_length, 0));
            for (size_t f = 0; f < n_inter_frames; f++)
                std::fill(init_data[f].begin(), init_data[f].end(), 40 + n_inter_frames * tid + f);
            cur_initializer->set_init_data(init_data);
            tid++;
        }

        if (!dot_filepath.empty())
        {
            std::ofstream file(dot_filepath);
            sequence_chain->export_dot(file);
        }

        for (auto& mod : sequence_chain->get_modules<module::Module>(false))
            for (auto& tsk : mod->tasks)
            {
                tsk->reset();
                tsk->set_debug(debug);
                tsk->set_debug_limit(16);
                tsk->set_stats(print_stats);
                tsk->set_fast(true);
            }

        std::atomic<unsigned int> counter(0);
        auto t_start = std::chrono::steady_clock::now();
        if (!step_by_step)
            sequence_chain->exec([&counter, n_exec]() { return ++counter >= n_exec; });
        else
        {
            do
            {
                for (size_t th = 0; th < (n_threads ? n_threads : 1); th++)
                    while (sequence_chain->exec_step(th))
                        ;
            } while (++counter < (n_exec / (n_threads ? n_threads : 1)));
        }
        std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;
        auto elapsed_time = duration.count() / 1000.f / 1000.f;
        std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;

        finalizer_list = sequence_chain->get_cloned_modules<module::Finalizer<uint8_t>>(finalizer);
    }
    else
    {
        runtime::Task* stage2_first = hybrid ? &(*incs_cpp2[0])("increment") : &(*incs_jl2[0])("increment");
        runtime::Task* stage2_last =
          hybrid ? &(*incs_cpp2[n_stage2 - 1])("increment") : &(*incs_jl2[n_stage2 - 1])("increment");

        pipeline_chain.reset(new runtime::Pipeline(
          initializer("initialize"),
          {
            // pipeline stage 0: Initializer (C++)
            { { &initializer("initialize") }, { &initializer("initialize") } },
            // pipeline stage 1: 3x Stateless_Julia incrementers
            { { &(*incs_jl1[0])("increment") }, { &(*incs_jl1[n_stage1 - 1])("increment") } },
            // pipeline stage 2: 3x Stateless_Julia (or C++ Incrementer in hybrid mode)
            { { stage2_first }, { stage2_last } },
            // pipeline stage 3: Finalizer (C++)
            { { &finalizer("finalize") }, {} },
          },
          {
            1,                         // stage 0 threads
            n_threads ? n_threads : 1, // stage 1 threads
            n_threads ? n_threads : 1, // stage 2 threads
            1                          // stage 3 threads
          },
          {
            buffer_size, // buffer between stages 0 and 1
            buffer_size, // buffer between stages 1 and 2
            buffer_size, // buffer between stages 2 and 3
          },
          {
            active_waiting,
            active_waiting,
            active_waiting,
          }));
        pipeline_chain->set_n_frames(n_inter_frames);

        size_t tid = 0;
        for (auto cur_initializer :
             pipeline_chain->get_stages()[0]->get_cloned_modules<module::Initializer<uint8_t>>(initializer))
        {
            std::vector<std::vector<uint8_t>> init_data(n_inter_frames, std::vector<uint8_t>(data_length, 0));
            for (size_t f = 0; f < n_inter_frames; f++)
                std::fill(init_data[f].begin(), init_data[f].end(), 40 + n_inter_frames * tid + f);
            cur_initializer->set_init_data(init_data);
            tid++;
        }

        if (!dot_filepath.empty())
        {
            std::ofstream file(dot_filepath);
            pipeline_chain->export_dot(file);
        }

        for (auto& mod : pipeline_chain->get_modules<module::Module>(false))
            for (auto& tsk : mod->tasks)
            {
                tsk->reset();
                tsk->set_debug(debug);
                tsk->set_debug_limit(16);
                tsk->set_stats(print_stats);
                tsk->set_fast(true);
            }

        std::atomic<unsigned int> counter(0);
        auto t_start = std::chrono::steady_clock::now();
        pipeline_chain->exec([&counter, n_exec]() { return ++counter >= n_exec; });
        std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;
        auto elapsed_time = duration.count() / 1000.f / 1000.f;
        std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;

        finalizer_list = pipeline_chain->get_stages()[pipeline_chain->get_stages().size() - 1]
                           ->get_cloned_modules<module::Finalizer<uint8_t>>(finalizer);
    }

    bool tests_passed = true;
    size_t tid = 0;
    for (auto cur_finalizer : finalizer_list)
    {
        for (size_t f = 0; f < n_inter_frames; f++)
        {
            const auto& final_data = cur_finalizer->get_final_data()[f];
            for (size_t d = 0; d < final_data.size(); d++)
            {
                auto expected = (int)((40 + n_inter_frames * tid + f + n_stage1 + n_stage2) % 256);
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

    if (print_stats)
    {
        std::cout << "#" << std::endl;
        if (force_sequence)
            tools::Stats::show(sequence_chain->get_modules_per_types(), true, false);
        else
        {
            auto stages = pipeline_chain->get_stages();
            for (size_t s = 0; s < stages.size(); s++)
            {
                const int st_threads = stages[s]->get_n_threads();
                std::cout << "#" << std::endl
                          << "# Pipeline stage " << s << " (" << st_threads << " thread(s)): " << std::endl;
                tools::Stats::show(stages[s]->get_tasks_per_types(), true, false);
            }
        }
    }

    if (force_sequence)
        sequence_chain->set_n_frames(1);
    else
    {
        pipeline_chain->set_n_frames(1);
        pipeline_chain->unbind_adaptors();
    }

    (*incs_jl1[0])["increment::in"].unbind(initializer["initialize::out"]);
    for (size_t s = 0; s < n_stage1 - 1; s++)
        (*incs_jl1[s + 1])["increment::in"].unbind((*incs_jl1[s])["increment::out"]);

    if (hybrid)
    {
        (*incs_cpp2[0])["increment::in"].unbind((*incs_jl1[n_stage1 - 1])["increment::out"]);
        for (size_t s = 0; s < n_stage2 - 1; s++)
            (*incs_cpp2[s + 1])["increment::in"].unbind((*incs_cpp2[s])["increment::out"]);
        finalizer["finalize::in"].unbind((*incs_cpp2[n_stage2 - 1])["increment::out"]);
    }
    else
    {
        (*incs_jl2[0])["increment::in"].unbind((*incs_jl1[n_stage1 - 1])["increment::out"]);
        for (size_t s = 0; s < n_stage2 - 1; s++)
            (*incs_jl2[s + 1])["increment::in"].unbind((*incs_jl2[s])["increment::out"]);
        finalizer["finalize::in"].unbind((*incs_jl2[n_stage2 - 1])["increment::out"]);
    }

    return test_results;
}

