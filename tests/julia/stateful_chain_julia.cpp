#include <algorithm>
#include <atomic>
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

int
main(int argc, char** argv)
{
    tools::Signal_handler::init();
    tools::Julia::init(256);

    option longopts[] = { { "n-threads", required_argument, NULL, 't' },
                          { "n-inter-frames", required_argument, NULL, 'f' },
                          { "sleep-time", required_argument, NULL, 's' },
                          { "data-length", required_argument, NULL, 'd' },
                          { "n-exec", required_argument, NULL, 'e' },
                          { "dot-filepath", required_argument, NULL, 'o' },
                          { "copy-mode", no_argument, NULL, 'c' },
                          { "print-stats", no_argument, NULL, 'p' },
                          { "step-by-step", no_argument, NULL, 'b' },
                          { "jl-safe", no_argument, NULL, 'J' },
                          { "jl-extern", no_argument, NULL, 'E' },
                          { "debug", no_argument, NULL, 'g' },
                          { "verbose", no_argument, NULL, 'v' },
                          { "help", no_argument, NULL, 'h' },
                          { NULL, 0, NULL, 0 } };

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
    bool jl_safe = false;
    bool jl_extern = false;

    while (1)
    {
        const int opt = getopt_long(argc, argv, "t:f:s:d:e:o:cpbgvhJE", longopts, 0);
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
            case 'J':
                jl_safe = true;
                break;
            case 'E':
                jl_extern = true;
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
                std::cout << "  -J, --jl-safe         "
                          << "Enable Julia safe mode (throw exceptions to C++)                      "
                          << "[" << (jl_safe ? "true" : "false") << "]" << std::endl;
                std::cout << "  -E, --jl-extern       "
                          << "Evaluate the Julia code from an external file                         "
                          << "[" << (jl_extern ? "true" : "false") << "]" << std::endl;
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

    std::cout << "#########################################" << std::endl;
    std::cout << "# Micro-benchmark: Stateful chain Julia #" << std::endl;
    std::cout << "#########################################" << std::endl;
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
    std::cout << "#   - jl_safe        = " << (jl_safe ? "true" : "false") << std::endl;
    std::cout << "#   - jl_extern      = " << (jl_extern ? "true" : "false") << std::endl;
    std::cout << "#   - debug          = " << (debug ? "true" : "false") << std::endl;
    std::cout << "#   - verbose        = " << (verbose ? "true" : "false") << std::endl;
    std::cout << "#" << std::endl;

    // modules creation
    module::Initializer<uint8_t> initializer(data_length);
    module::Finalizer<uint8_t> finalizer(data_length);

    // 1. Prototype for 5 single-task Stateful_Julia modules with persistent StepState(1, 0)
    module::Stateful_Julia incr;
    runtime::Task& t = incr.create_task("increment_stateful");
    incr.create_socket_in<uint8_t>(t, "in", data_length);
    incr.create_socket_out<uint8_t>(t, "out", data_length);
    incr.create_constant<uint32_t>(t, sleep_time_us * 1000);
    if (jl_extern)
    {
        incr.create_state_file("StepState(1, 0)", false, "../tests/julia/states.jl");
        incr.create_codelet_file(t, "../tests/julia/codelets.jl");
    }
    else
    {
        incr.create_state("StepState(1, 0)", false, R""""(
            mutable struct StepState
                step::UInt8
                calls::UInt32
            end
        )"""");
        incr.create_codelet(t, R""""(
            function increment_stateful(stt_step, sck_in, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
                if cst_wait_time_ns > 0
                    start_incr = time_ns()
                end

                if rnt_frame_id == 0
                    stt_step.calls += one(typeof(stt_step.calls))
                end

                step_val = eltype(sck_in)(stt_step.step)
                for n in eachindex(sck_in)
                    sck_out[n] = sck_in[n] + step_val
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
    }

    std::vector<std::shared_ptr<module::Stateful_Julia>> incs(5);
    for (size_t s = 0; s < incs.size(); s++)
    {
        incs[s].reset(incr.clone());
        incs[s]->set_custom_name("StInc" + std::to_string(s));
        incs[s]->set_jl_safety(jl_safe);
    }

    // 2. A two-task Stateful_Julia module ('memorize' -> 'produce') sharing a common SharedBufferState
    module::Stateful_Julia mem_prod;
    mem_prod.set_custom_name("StMemProd");
    mem_prod.set_jl_safety(jl_safe);
    runtime::Task& t_mem = mem_prod.create_task("memorize");
    mem_prod.create_socket_in<uint8_t>(t_mem, "in", data_length);
    mem_prod.create_socket_out<uint8_t>(t_mem, "sync", 1);
    mem_prod.create_constant<uint32_t>(t_mem, sleep_time_us * 1000);

    runtime::Task& t_prod = mem_prod.create_task("produce");
    mem_prod.create_socket_in<uint8_t>(t_prod, "sync", 1);
    mem_prod.create_socket_out<uint8_t>(t_prod, "out", data_length);
    mem_prod.create_constant<uint32_t>(t_prod, 0);

    const std::string buf_ctor =
      "SharedBufferState(zeros(UInt8, " + std::to_string(data_length * n_inter_frames) + "))";
    if (jl_extern)
    {
        mem_prod.create_state_file(buf_ctor, false, "../tests/julia/states.jl");
        mem_prod.create_codelet_file(t_mem, "../tests/julia/codelets.jl");
        mem_prod.create_codelet_file(t_prod, "../tests/julia/codelets.jl");
    }
    else
    {
        mem_prod.create_state(buf_ctor, false, R""""(
            mutable struct SharedBufferState
                buf::Vector{UInt8}
            end
        )"""");
        mem_prod.create_codelet(t_mem, R""""(
            function memorize(stt_buf, sck_in, sck_sync, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
                if cst_wait_time_ns > 0
                    start_mem = time_ns()
                end

                frame_len = div(length(sck_in), Int(rnt_n_frames_per_wave))
                offset = Int(rnt_frame_id) * frame_len
                if length(stt_buf.buf) < offset + length(sck_in)
                    resize!(stt_buf.buf, offset + length(sck_in))
                end

                for n in eachindex(sck_in)
                    stt_buf.buf[offset + n] = sck_in[n] + one(eltype(sck_in))
                end
                sck_sync[1] = one(eltype(sck_sync))

                if cst_wait_time_ns != 0
                    stop_mem = time_ns()
                    elapse_mem = stop_mem - start_mem

                    while elapse_mem < cst_wait_time_ns
                        elapse_mem = time_ns() - start_mem
                    end
                end

                return 0
            end
        )"""");
        mem_prod.create_codelet(t_prod, R""""(
            function produce(stt_buf, sck_sync, sck_out, cst_wait_time_ns, rnt_frame_id, rnt_n_frames_per_wave)
                frame_len = div(length(sck_out), Int(rnt_n_frames_per_wave))
                offset = Int(rnt_frame_id) * frame_len
                for n in eachindex(sck_out)
                    sck_out[n] = stt_buf.buf[offset + n]
                end
                return 0
            end
        )"""");
    }

    const size_t total_increments = incs.size() + 1;

    // sockets binding
    (*incs[0])["increment_stateful::in"] = initializer["initialize::out"];
    for (size_t s = 0; s < incs.size() - 1; s++)
        (*incs[s + 1])["increment_stateful::in"] = (*incs[s])["increment_stateful::out"];
    mem_prod["memorize::in"] = (*incs[incs.size() - 1])["increment_stateful::out"];
    mem_prod["produce::sync"] = mem_prod["memorize::sync"];
    finalizer["finalize::in"] = mem_prod["produce::out"];

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
        {
            size_t init_val = (tid * n_inter_frames + f) % (256 - total_increments);
            std::fill(init_data[f].begin(), init_data[f].end(), init_val);
        }
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
            tsk->set_debug(debug);
            tsk->set_debug_limit(16);
            tsk->set_stats(print_stats);
            tsk->set_fast(true);
        }

    if (verbose)
    {
        std::cout << std::endl;
        std::cout << "Helper information:" << std::endl;
        std::cout << "-------------------" << std::endl;
        tools::help(initializer);
        for (size_t s = 0; s < incs.size(); s++)
            tools::help(*incs[s]);
        tools::help(mem_prod);
        tools::help(finalizer);
    }

    std::atomic<unsigned int> counter(0);
    auto t_start = std::chrono::steady_clock::now();
    if (!step_by_step)
    {
        sequence_chain.exec(
          [&counter, n_exec]()
          {
              return ++counter >= n_exec;
          });
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

    size_t chain_sleep_time = total_increments * sleep_time_us * 1000;
    auto theoretical_time = (chain_sleep_time * n_exec * n_inter_frames) / 1000.f / 1000.f / n_threads;
    std::cout << "Sequence theoretical time: " << theoretical_time << " ms" << std::endl;

    // 1. Verification of the sequence output data
    bool tests_passed = true;
    tid = 0;
    for (auto cur_finalizer : sequence_chain.get_cloned_modules<module::Finalizer<uint8_t>>(finalizer))
    {
        for (size_t f = 0; f < n_inter_frames; f++)
        {
            const auto& final_data = cur_finalizer->get_final_data()[f];
            for (size_t d = 0; d < final_data.size(); d++)
            {
                auto expected = (int)(total_increments + ((tid * n_inter_frames + f) % (256 - total_increments)));
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

    // 2. Verification of the persistent Julia state across thread clones
    const size_t expected_total_calls = n_threads + (!step_by_step ? counter.load() : counter.load() * n_threads);
    for (size_t s = 0; s < incs.size(); s++)
    {
        size_t total_calls = 0;
        size_t clone_id = 0;
        for (auto cur_inc : sequence_chain.get_cloned_modules<module::Stateful_Julia>(*incs[s]))
        {
            if (cur_inc->get_states().empty() || cur_inc->get_states()[0] == nullptr)
            {
                tests_passed = false;
                break;
            }
            jl_value_t* calls_val = jl_get_nth_field(cur_inc->get_states()[0], 1);
            uint32_t clone_calls = jl_unbox_uint32(calls_val);
            uint32_t expected_clone_calls = 1 + cur_inc->tasks[0]->get_n_calls();
            if (clone_calls != expected_clone_calls)
            {
                std::cout << "# state verification failed for StInc" << s << " (clone " << clone_id
                          << "): clone_calls = " << clone_calls << " != expected_clone_calls = " << expected_clone_calls
                          << std::endl;
                tests_passed = false;
            }
            total_calls += clone_calls;
            clone_id++;
        }
        // Each clone ran once during the warmup (n_threads executions) plus the main execution loop
        if (total_calls != expected_total_calls)
        {
            std::cout << "# state verification failed for StInc" << s << ": total_calls = " << total_calls
                      << " != expected_total_calls = " << expected_total_calls << std::endl;
            tests_passed = false;
        }
    }

    // Verify StreamPU type hierarchy (Stateless vs Stateful) for Stateless_Julia and Stateful_Julia
    module::Stateless_Julia dummy_stateless;
    module::Module* m_stateless = &dummy_stateless;
    module::Module* m_stateful = &mem_prod;
    if (!m_stateless->is_stateless() || m_stateless->is_stateful() ||
        dynamic_cast<module::Stateless*>(m_stateless) == nullptr ||
        dynamic_cast<module::Stateful*>(m_stateless) != nullptr ||
        !m_stateful->is_stateful() || m_stateful->is_stateless() ||
        dynamic_cast<module::Stateful*>(m_stateful) == nullptr ||
        dynamic_cast<module::Stateless*>(m_stateful) != nullptr)
    {
        std::cout << "# Type hierarchy check failed for Stateless_Julia / Stateful_Julia!" << std::endl;
        tests_passed = false;
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
        tools::Stats::show(sequence_chain.get_modules_per_types(), true, false);
    }

    // sockets unbinding
    sequence_chain.set_n_frames(1);

    (*incs[0])["increment_stateful::in"].unbind(initializer["initialize::out"]);
    for (size_t s = 0; s < incs.size() - 1; s++)
        (*incs[s + 1])["increment_stateful::in"].unbind((*incs[s])["increment_stateful::out"]);
    mem_prod["memorize::in"].unbind((*incs[incs.size() - 1])["increment_stateful::out"]);
    mem_prod["produce::sync"].unbind(mem_prod["memorize::sync"]);
    finalizer["finalize::in"].unbind(mem_prod["produce::out"]);

    return test_results;
}

