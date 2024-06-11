#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <streampu.hpp>
using namespace spu;
using namespace spu::runtime;

enum class tsk_e : uint8_t
{
    initialize,
    read,
    relay,
    relayf,
    increment,
    incrementf,
    finalize,
    write,
};

static std::map<std::string, tsk_e> str_2_tsk = {
    { "initialize", tsk_e::initialize }, { "init", tsk_e::initialize },       { "read", tsk_e::read },
    { "relay", tsk_e::relay },           { "relayf", tsk_e::relayf },         { "increment", tsk_e::increment },
    { "incr", tsk_e::increment },        { "incrementf", tsk_e::incrementf }, { "incrf", tsk_e::incrementf },
    { "finalize", tsk_e::finalize },     { "fin", tsk_e::finalize },          { "write", tsk_e::write },
};

// clang-format off
//              task              tsk ids              sck IN ids           sck OUT ids
static std::map<tsk_e, std::tuple<std::vector<size_t>, std::vector<size_t>, std::vector<size_t>>> tsk_2_ids = {
    { tsk_e::initialize, {  { 0    }, {      }, { 0    } } },
    { tsk_e::read,       {  { 0    }, {      }, { 0, 1 } } },
    { tsk_e::relay,      {  { 0    }, { 0    }, { 1    } } },
    { tsk_e::relayf,     {  { 1    }, { 0    }, { 0    } } },
    { tsk_e::increment,  {  { 0    }, { 0    }, { 1    } } },
    { tsk_e::incrementf, {  { 1    }, { 0    }, { 0    } } },
    { tsk_e::finalize,   {  { 0    }, { 0    }, {      } } },
    { tsk_e::write,      {  { 0, 1 }, { 0, 1 }, {      } } },
};
// clang-format on

// Error message variable
std::stringstream message;

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

std::vector<std::string>
split_string(std::string s, const std::string delimiter = "_")
{
    std::vector<std::string> tokens;

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);

    return tokens;
}

// Functions to parse arguments
void
parse_int_string(std::string& vector_param, std::vector<size_t>& vector)
{
    std::vector<std::string> tokens = split_string(vector_param, ",");
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        std::string token = tokens[i];
        token.erase(remove_if(token.begin(), token.end(), isspace), token.end());
        vector.push_back(std::atoi(token.c_str()));
    }
}

std::pair<tsk_e, int>
extract_tsk_type(const std::string& label)
{
    tsk_e tsk = tsk_e::relay;
    int duration = -1;

    std::vector<std::string> tokens = split_string(label);

    if (tokens.size())
    {
        size_t i = 0;
        while (i < tokens.size())
        {
            std::string token = tokens[i];
            token.erase(remove_if(token.begin(), token.end(), isspace), token.end());
            if (str_2_tsk.find(token) != str_2_tsk.end())
            {
                tsk = str_2_tsk[token];
            }
            else if (std::atoi(token.c_str()))
            {
                duration = std::atoi(token.c_str());
            }
            else
            {
                if (!token.empty())
                {
                    message << "Current token is incompatible (token = '" << token << "').";
                    throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
                }
            }
            i++;
        };
    }
    else
    {
        message << "tokens.size() has to be higher than 0.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    return std::make_pair(tsk, duration);
}

void
parse_tsk_types(std::string& tsk_types_param, std::vector<std::vector<std::pair<tsk_e, int>>>& tsk_types)
{
    size_t i = 0;
    size_t sta = 0;
    std::string tmp;
    while (i < tsk_types_param.size())
    {
        if (tsk_types_param[i] == '(' && tsk_types_param[i + 1] != '(')
        {
            tsk_types.push_back({});
            tmp.clear();
        }
        else if (tsk_types_param[i] == ',' && tsk_types_param[i + 1] != '(')
        {
            tsk_types[sta].push_back(extract_tsk_type(tmp));
            tmp.clear();
        }
        else if (tsk_types_param[i] == ')')
        {
            if (tsk_types_param[i + 1] != ')')
            {
                tsk_types[sta].push_back(extract_tsk_type(tmp));
                tmp.clear();
                sta++;
            }
        }
        else
            tmp.push_back(tsk_types_param[i]);

        i++;
    }
}

void
parse_tsk_types_sta(std::string& sck_type_sta_param, std::vector<std::pair<tsk_e, int>>& tsk_types)
{
    size_t i = 0;
    std::string tmp;
    while (i < sck_type_sta_param.size())
    {
        if (sck_type_sta_param[i] == '(')
            tmp.clear();
        else if (sck_type_sta_param[i] == ',' || sck_type_sta_param[i] == ')')
        {
            tsk_types.push_back(extract_tsk_type(tmp));
            tmp.clear();
        }
        else
            tmp.push_back(sck_type_sta_param[i]);
        i++;
    }
}

int
main(int argc, char** argv)
{
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
                          { "help", no_argument, NULL, 'h' },
                          { "tsk-per-sta", no_argument, NULL, 'n' },
                          { "tsk-types", no_argument, NULL, 'r' },
                          { "tsk-types-sta", no_argument, NULL, 'R' },
#ifdef SPU_HWLOC
                          { "pinning-policy", no_argument, NULL, 'P' },
#endif
                          { 0 } };

    std::string n_threads_param;
    std::vector<size_t> n_threads;
    size_t n_inter_frames = 1;
    size_t sleep_time_us = 5;
    size_t data_length = 2048;
    size_t n_exec = 0;
    size_t buffer_size = 16;
    std::string dot_filepath;
    std::string in_filepath;
    std::string out_filepath = "file.out";
    bool no_copy_mode = true;
    bool print_stats = false;
    bool step_by_step = false;
    bool debug = false;
    bool force_sequence = false;
    bool active_waiting = false;
    std::string tsk_per_sta_param;
    std::vector<size_t> tsk_per_sta;
    std::string tsk_types_param;
    std::vector<std::vector<std::pair<tsk_e, int>>> tsk_types;
    std::string tsk_types_sta_param;
    std::vector<std::pair<tsk_e, int>> tsk_types_sta;
    std::string pinning_policy;
    std::string check_task;

    while (1)
    {
        const int opt = getopt_long(argc, argv, "t:f:s:d:e:u:o:i:j:n:r:R:P:cpbgqwh", longopts, 0);
        if (opt == -1) break;
        switch (opt)
        {
            case 't':
                n_threads_param = std::string(optarg);
                parse_int_string(n_threads_param, n_threads);
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
            case 'n':
                tsk_per_sta_param = std::string(optarg);
                parse_int_string(tsk_per_sta_param, tsk_per_sta);
                break;
            case 'r':
                tsk_types_param = std::string(optarg);
                parse_tsk_types(tsk_types_param, tsk_types);
                break;
            case 'R':
                tsk_types_sta_param = std::string(optarg);
                parse_tsk_types_sta(tsk_types_sta_param, tsk_types_sta);
                break;
#ifdef SPU_HWLOC
            case 'P':
                pinning_policy = std::string(optarg);
                break;
#endif
            case 'h':
                std::cout << "usage: " << argv[0] << " [options]" << std::endl;
                std::cout << std::endl;
                std::cout << "  -t, --n-threads          "
                          << "Number of threads to run in parallel for each stage                   "
                             "["
                          << (n_threads_param.empty() ? "empty" : "\"" + n_threads_param + "\"") << "]" << std::endl;
                std::cout << "  -f, --n-inter-frames     "
                          << "Number of frames to process in one task                               "
                          << "[" << n_inter_frames << "]" << std::endl;
                std::cout << "  -s, --sleep-time         "
                          << "Sleep time duration in one task (microseconds)                        "
                          << "[" << sleep_time_us << "]" << std::endl;
                std::cout << "  -d, --data-length        "
                          << "Size of data to process in one task (in bytes)                        "
                          << "[" << data_length << "]" << std::endl;
                std::cout << "  -e, --n-exec             "
                          << "Number of executions (0 means -> never stop because of this counter)  "
                          << "[" << n_exec << "]" << std::endl;
                std::cout << "  -u, --buffer-size        "
                          << "Size of the buffer between the different stages of the pipeline       "
                          << "[" << data_length << "]" << std::endl;
                std::cout << "  -o, --dot-filepath       "
                          << "Path to dot output file                                               "
                          << "[" << (dot_filepath.empty() ? "empty" : "\"" + dot_filepath + "\"") << "]" << std::endl;
                std::cout << "  -i, --in-filepath        "
                          << "Path to the input file (used to generate bits of the chain)           "
                          << "[" << (in_filepath.empty() ? "empty" : "\"" + in_filepath + "\"") << "]" << std::endl;
                std::cout << "  -j, --out-filepath       "
                          << "Path to the output file (written at the end of the chain)             "
                          << "[" << (out_filepath.empty() ? "empty" : "\"" + out_filepath + "\"") << "]" << std::endl;
                std::cout << "  -c, --copy-mode          "
                          << "Enable to copy data in sequence (performance will be reduced)         "
                          << "[" << (no_copy_mode ? "false" : "true") << "]" << std::endl;
                std::cout << "  -b, --step-by-step       "
                          << "Enable step-by-step sequence execution (performance will be reduced)  "
                          << "[" << (step_by_step ? "true" : "false") << "]" << std::endl;
                std::cout << "  -p, --print-stats        "
                          << "Enable to print per task statistics (performance will be reduced)     "
                          << "[" << (print_stats ? "true" : "false") << "]" << std::endl;
                std::cout << "  -g, --debug              "
                          << "Enable task debug mode (print socket data)                            "
                          << "[" << (debug ? "true" : "false") << "]" << std::endl;
                std::cout << "  -q, --force-sequence     "
                          << "Force sequence instead of pipeline                                    "
                          << "[" << (force_sequence ? "true" : "false") << "]" << std::endl;
                std::cout << "  -w, --active-waiting     "
                          << "Enable active waiting in the pipeline synchronizations                "
                          << "[" << (active_waiting ? "true" : "false") << "]" << std::endl;
                std::cout << "  -n, --tsk-per-sta        "
                          << "The number of tasks on each stage of the pipeline                     "
                          << "[" << (tsk_per_sta_param.empty() ? "empty" : "\"" + tsk_per_sta_param + "\"") << "]"
                          << std::endl;
                std::cout << "  -r, --tsk-types          "
                          << "The socket type of each task (SFWD or SIO)                            "
                          << "[" << (tsk_types_param.empty() ? "empty" : "\"" + tsk_types_param + "\"") << "]"
                          << std::endl;
                std::cout << "  -R, --tsk-types-sta      "
                          << "The socket type of tasks on each stage (SFWD or SIO)                  "
                          << "[" << (tsk_types_sta_param.empty() ? "empty" : "\"" + tsk_types_sta_param + "\"") << "]"
                          << std::endl;
#ifdef SPU_HWLOC
                std::cout << "  -P, --pinning-policy     "
                          << "Pinning policy for pipeline execution                                 "
                          << "[" << (pinning_policy.empty() ? "empty" : "\"" + pinning_policy + "\"") << "]"
                          << std::endl;
#endif
                std::cout << "  -h, --help               "
                          << "This help                                                             "
                          << "[false]" << std::endl;
                exit(0);
                break;
            default:
                break;
        }
    }

#ifdef SPU_HWLOC
    if (!pinning_policy.empty()) tools::Thread_pinning::init();
#endif

    // Checking for errors
    if (!tsk_types_sta.empty() && !tsk_types.empty())
    {
        message << "You have to select only one parameter for socket type ('-r' exclusive or '-R').";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    // Building the socket type vector in case of stage socket
    for (size_t i = 0; i < tsk_types_sta.size(); ++i)
    {
        tsk_types.push_back({});
        for (size_t j = 0; j < tsk_per_sta[i]; ++j)
            tsk_types[i].push_back(tsk_types_sta[i]);
    }

    size_t n_tsk = 0;
    tsk_per_sta.resize(tsk_types.size());
    for (size_t i = 0; i < tsk_per_sta.size(); i++)
    {
        tsk_per_sta[i] = tsk_types[i].size();
        n_tsk += tsk_types[i].size();
    }

    if (n_tsk < 2)
    {
        message << "The chain should have more than one task (n_tsk = '" << n_tsk << "').";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    std::vector<std::pair<tsk_e, int>> tsk_types_1d;
    for (size_t i = 0; i < tsk_types.size(); i++)
        for (size_t j = 0; j < tsk_types[i].size(); j++)
            tsk_types_1d.push_back(tsk_types[i][j]);

    if (n_threads.size() == 0) n_threads.push_back(1);

    std::cout << "#####################################" << std::endl;
    std::cout << "# Micro-benchmark: Generic pipeline #" << std::endl;
    std::cout << "#####################################" << std::endl;
    std::cout << "#" << std::endl;

    std::cout << "# Command line arguments:" << std::endl;
    std::cout << "#   - n_threads      = " << (n_threads_param.empty() ? "[empty] (def: 1)" : n_threads_param.c_str())
              << std::endl;
    std::cout << "#   - tsk_per_sta    = " << (tsk_per_sta_param.empty() ? "[empty]" : tsk_per_sta_param.c_str())
              << std::endl;
    std::cout << "#   - tsk_types      = " << (tsk_types_param.empty() ? "[empty]" : tsk_types_param.c_str())
              << std::endl;
    std::cout << "#   - tsk_types_sta  = " << (tsk_types_sta_param.empty() ? "[empty]" : tsk_types_sta_param.c_str())
              << std::endl;
#ifdef SPU_HWLOC
    std::cout << "#   - pinning_policy = " << (pinning_policy.empty() ? "[empty]" : pinning_policy.c_str())
              << std::endl;
#endif
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
    std::cout << "#" << std::endl;

    if (!force_sequence && !no_copy_mode)
        std::clog << rang::tag::warning << "'no_copy_mode' has no effect with pipeline (it is always enable)"
                  << std::endl;
    if (!force_sequence && step_by_step)
        std::clog << rang::tag::warning << "'step_by_step' is not available with pipeline" << std::endl;

    // modules creation
    std::vector<std::shared_ptr<module::Module>> modules(n_tsk, nullptr);
    size_t tas = 0;
    for (auto sta : tsk_types)
    {
        for (auto tst : sta)
        {
            switch (tst.first)
            {
                case tsk_e::initialize:
                {
                    if (tas != 0)
                    {
                        message << "An 'Initializer' can only be at the begining of the chain (tas = '" << tas << "').";
                        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
                    }
                    if (tst.second != -1)
                    {
                        message << "An 'Initializer' can't have a duration (tas = '" << tas << "').";
                        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
                    }
                    auto initializer = new module::Initializer<uint8_t>(data_length);
                    modules[tas].reset(initializer);
                    initializer->set_custom_name("Init" + std::to_string(tas));
                    break;
                }
                case tsk_e::read:
                {
                    if (tas != 0)
                    {
                        message << "A 'Source' can only be at the begining of the chain (tas = '" << tas << "').";
                        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
                    }
                    if (tst.second != -1)
                    {
                        message << "A 'Source' can't have a duration (tas = '" << tas << "').";
                        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
                    }
                    const bool auto_reset = false;
                    auto source = new module::Source_user_binary<uint8_t>(data_length, in_filepath, auto_reset);
                    modules[tas].reset(source);
                    source->set_custom_name("Reader" + std::to_string(tas));
                    break;
                }
                case tsk_e::relay:
                case tsk_e::relayf:
                {
                    if (tas == 0 || tas == n_tsk - 1)
                    {
                        message << "A 'Relayer' can't be at the begining or at the end of the chain (tas = '" << tas
                                << "').";
                        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
                    }
                    auto relayer = new module::Relayer<uint8_t>(data_length);
                    modules[tas].reset(relayer);
                    size_t sleep_duration_us = (tst.second == -1) ? sleep_time_us : tst.second;
                    relayer->set_ns(sleep_duration_us * 1000);
                    if (tst.second != -1)
                        relayer->set_custom_name("Rly" + std::to_string(tas) + "_d" +
                                                 std::to_string(sleep_duration_us));
                    else
                        relayer->set_custom_name("Relayer" + std::to_string(tas));
                    break;
                }
                case tsk_e::increment:
                case tsk_e::incrementf:
                {
                    if (tas == 0 || tas == n_tsk - 1)
                    {
                        message << "An 'Incrementer' can't be at the begining or at the end of the chain (tas = '"
                                << tas << "').";
                        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
                    }
                    auto incrementer = new module::Incrementer<uint8_t>(data_length);
                    modules[tas].reset(incrementer);
                    size_t sleep_duration_us = (tst.second == -1) ? sleep_time_us : tst.second;
                    incrementer->set_ns(sleep_duration_us * 1000);
                    if (tst.second != -1)
                        incrementer->set_custom_name("Incr" + std::to_string(tas) + "_d" +
                                                     std::to_string(sleep_duration_us));
                    else
                        incrementer->set_custom_name("Incr" + std::to_string(tas));
                    break;
                }
                case tsk_e::finalize:
                {
                    if (tas != n_tsk - 1)
                    {
                        message << "A 'Finalizer' can only be at the end of the chain (tas = '" << tas << "').";
                        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
                    }
                    if (tst.second != -1)
                    {
                        message << "A 'Finalizer' can't have a duration (tas = '" << tas << "').";
                        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
                    }
                    auto finalizer = new module::Finalizer<uint8_t>(data_length);
                    modules[tas].reset(finalizer);
                    finalizer->set_custom_name("Fin" + std::to_string(tas));
                    break;
                }
                case tsk_e::write:
                {
                    if (tas != n_tsk - 1)
                    {
                        message << "A 'Sink' can only be at the end of the chain (tas = '" << tas << "').";
                        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
                    }
                    if (tst.second != -1)
                    {
                        message << "A 'Sink' can't have a duration (tas = '" << tas << "').";
                        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
                    }
                    auto sink = new module::Sink_user_binary<uint8_t>(data_length, out_filepath);
                    modules[tas].reset(sink);
                    sink->set_custom_name("Writer" + std::to_string(tas));
                    break;
                }
                default:
                    message << "Unsupported case (tas = '" << tas << "').";
                    throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
            };
            tas++;
        }
    }

    // sockets binding
    for (size_t t = 0; t < tsk_types_1d.size() - 1; t++)
    {
        size_t tskid_out = std::get<0>(tsk_2_ids[tsk_types_1d[t].first])[0];
        size_t sckid_out = std::get<2>(tsk_2_ids[tsk_types_1d[t].first])[0];
        size_t tskid_in = std::get<0>(tsk_2_ids[tsk_types_1d[t + 1].first])[0];
        size_t sckid_in = std::get<1>(tsk_2_ids[tsk_types_1d[t + 1].first])[0];

        if (tsk_types_1d[t + 1].first == tsk_e::write && tsk_types_1d[0].first == tsk_e::read)
            tskid_in = std::get<0>(tsk_2_ids[tsk_types_1d[t + 1].first])[1];

        (*modules[t + 1].get())[tskid_in][sckid_in] = (*modules[t].get())[tskid_out][sckid_out];
    }
    if (tsk_types_1d[n_tsk - 1].first == tsk_e::write && tsk_types_1d[0].first == tsk_e::read)
        (*modules[n_tsk - 1].get())[1][1] = (*modules[0].get())[0][1];

    std::unique_ptr<runtime::Sequence> sequence_chain;
    std::unique_ptr<runtime::Pipeline> pipeline_chain;
    std::vector<module::Finalizer<uint8_t>*> finalizer_list;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////// SEQUENCE EXEC //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (force_sequence)
    {
        size_t tskid_last =
          (tsk_types_1d[n_tsk - 1].first == tsk_e::write && tsk_types_1d[0].first == tsk_e::read) ? 1 : 0;
        sequence_chain.reset(
          new runtime::Sequence((*modules[0].get())[0], (*modules[n_tsk - 1].get())[tskid_last], n_threads[0]));
        sequence_chain->set_n_frames(n_inter_frames);
        sequence_chain->set_no_copy_mode(no_copy_mode);

        if (!dot_filepath.empty())
        {
            std::ofstream file(dot_filepath);
            sequence_chain->export_dot(file);
        }

        // configuration of the sequence tasks
        for (auto& mod : sequence_chain->get_modules<module::Module>(false))
            for (auto& tsk : mod->tasks)
            {
                tsk->reset();
                tsk->set_debug(debug);       // disable the debug mode
                tsk->set_debug_limit(16);    // display only the 16 first bits if the debug mode is enabled
                tsk->set_stats(print_stats); // enable the statistics
                tsk->set_fast(true);         // enable the fast mode (= disable the useless verifs in the tasks)
            }

        // prepare input data in case of initializer first
        if (tsk_types[0][0].first == tsk_e::initialize)
        {
            auto tid = 0;
            for (auto cur_initializer : sequence_chain.get()->get_cloned_modules<module::Initializer<uint8_t>>(
                   *((module::Initializer<uint8_t>*)(modules[0].get()))))
            {
                std::vector<std::vector<uint8_t>> init_data(n_inter_frames, std::vector<uint8_t>(data_length, 0));
                for (size_t f = 0; f < n_inter_frames; f++)
                    std::fill(init_data[f].begin(), init_data[f].end(), 40 + tid * n_inter_frames + f);
                cur_initializer->set_init_data(init_data);
                tid++;
            }
        }

        auto t_start = std::chrono::steady_clock::now();
        std::atomic<unsigned int> counter(0);
        if (!step_by_step)
            sequence_chain->exec([&counter, n_exec]() { return n_exec && (++counter >= n_exec); });
        else
        {
            do
            {
                try
                {
                    for (size_t tid = 0; tid < n_threads[0]; tid++)
                        while (sequence_chain->exec_step(tid))
                            ;
                }
                catch (tools::processing_aborted&)
                { /* do nothing */
                }
            } while ((!n_exec || ++counter < (n_exec / n_threads[0])) &&
                     ((tsk_types_1d[0].first == tsk_e::read)
                        ? !(*((module::Source_user_binary<uint8_t>*)(modules[0].get()))).is_done()
                        : true));
        }
        std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

        auto elapsed_time = duration.count() / 1000.f / 1000.f;
        std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;

        if (tsk_types_1d[n_tsk - 1].first == tsk_e::finalize)
            finalizer_list = sequence_chain.get()->get_cloned_modules<module::Finalizer<uint8_t>>(
              *((module::Finalizer<uint8_t>*)modules[n_tsk - 1].get()));
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////// PIPELINE EXEC //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    else
    {
        // create the stages
        std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> stages;
        tas = 0;
        for (size_t s = 0; s < tsk_types.size(); ++s)
        {
            size_t n_tsk_cur_sta = tsk_types[s].size();
            size_t tskid_first = std::get<0>(tsk_2_ids[tsk_types[s][0].first])[0];
            if (tsk_types[s][0].first == tsk_e::write && tsk_types[0][0].first == tsk_e::read) tskid_first = 1;

            size_t tskid_last = std::get<0>(tsk_2_ids[tsk_types[s][n_tsk_cur_sta - 1].first])[0];
            if (tsk_types[s][n_tsk_cur_sta - 1].first == tsk_e::write && tsk_types[0][0].first == tsk_e::read)
                tskid_last = 1;

            spu::runtime::Task* first_stage_task = &(*modules[tas].get())[tskid_first];
            spu::runtime::Task* last_stage_task = &(*modules[tas + n_tsk_cur_sta - 1].get())[tskid_last];

            if (tsk_types[s][n_tsk_cur_sta - 1].first == tsk_e::write && tsk_types[0][0].first == tsk_e::read &&
                n_tsk_cur_sta > 1)
                stages.push_back({ { first_stage_task, last_stage_task }, {} });
            else
                stages.push_back({ { first_stage_task }, { last_stage_task } });
            tas += n_tsk_cur_sta;
        }

        // buffer vector
        std::vector<size_t> pool_buff;
        for (size_t s = 0; s < tsk_types.size() - 1; ++s)
            pool_buff.push_back(buffer_size);

        // waiting type
        std::vector<bool> wait_vect;
        for (size_t s = 0; s < tsk_types.size() - 1; ++s)
            wait_vect.push_back(active_waiting);

        std::vector<bool> enable_pin = std::vector<bool>(tsk_types.size(), pinning_policy.empty() ? false : true);
        pipeline_chain.reset(new runtime::Pipeline(
          (*modules[0].get())[0], stages, n_threads, pool_buff, wait_vect, enable_pin, pinning_policy));
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

        // prepare input data in case of increment
        if (tsk_types[0][0].first == tsk_e::initialize)
        {
            auto tid = 0;
            for (auto cur_initializer :
                 pipeline_chain.get()->get_stages()[0]->get_cloned_modules<module::Initializer<uint8_t>>(
                   *((module::Initializer<uint8_t>*)(modules[0].get()))))
            {
                std::vector<std::vector<uint8_t>> init_data(n_inter_frames, std::vector<uint8_t>(data_length, 0));
                for (size_t f = 0; f < n_inter_frames; f++)
                    std::fill(init_data[f].begin(), init_data[f].end(), 40 + tid * n_inter_frames + f);
                cur_initializer->set_init_data(init_data);
                tid++;
            }
        }

        std::atomic<unsigned int> counter(0);
        auto t_start = std::chrono::steady_clock::now();
        pipeline_chain->exec([&counter, n_exec]() { return n_exec && (++counter >= n_exec); });
        std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

        auto elapsed_time = duration.count() / 1000.f / 1000.f;
        std::cout << "Pipeline elapsed time: " << elapsed_time << " ms" << std::endl;

        if (tsk_types_1d[n_tsk - 1].first == tsk_e::finalize)
            finalizer_list = pipeline_chain.get()
                               ->get_stages()[pipeline_chain.get()->get_stages().size() - 1]
                               ->get_cloned_modules<module::Finalizer<uint8_t>>(
                                 *((module::Finalizer<uint8_t>*)modules[n_tsk - 1].get()));
    }

    // count the number of incrementers in the chain
    size_t n_incr_count = 0;
    for (auto& t : tsk_types_1d)
        if (t.first == tsk_e::increment || t.first == tsk_e::incrementf) n_incr_count++;

    // verification of the pipeline/sequence execution
    unsigned int test_results = 1;
    if (tsk_types_1d[0].first == tsk_e::initialize && tsk_types_1d[n_tsk - 1].first == tsk_e::finalize &&
        (n_threads[0] == n_threads[n_threads.size() - 1] || force_sequence))
    {
        bool tests_passed = true;
        auto tid = 0;

        for (auto cur_finalizer : finalizer_list)
        {
            for (size_t f = 0; f < n_inter_frames; f++)
            {
                const auto& final_data = cur_finalizer->get_final_data()[f];
                for (size_t d = 0; d < final_data.size(); d++)
                {
                    auto expected = (int)(40 + n_incr_count + n_inter_frames * tid + f);
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
            std::cout << "# " << rang::style::bold << rang::fg::green << "Tests passed!" << rang::style::reset
                      << std::endl;
        else
            std::cout << "# " << rang::style::bold << rang::fg::red << "Tests failed :-(" << rang::style::reset
                      << std::endl;

        test_results = !tests_passed;
    }
    else if (tsk_types_1d[0].first == tsk_e::read && tsk_types_1d[n_tsk - 1].first == tsk_e::write && n_incr_count == 0)
    {
        size_t in_filesize = filesize(in_filepath.c_str());
        size_t n_frames = ((int)std::ceil((float)(in_filesize * 8) / (float)(data_length * n_inter_frames)));
        auto theoretical_time =
          (n_frames * ((modules.size()) * sleep_time_us * 1000) * n_inter_frames) / 1000.f / 1000.f;
        std::cout << "Sequence theoretical time: " << theoretical_time << " ms" << std::endl;
        // verification of the sequence execution
        bool tests_passed = compare_files(in_filepath, out_filepath);
        if (tests_passed)
            std::cout << "# " << rang::style::bold << rang::fg::green << "Tests passed!" << rang::style::reset
                      << std::endl;
        else
            std::cout << "# " << rang::style::bold << rang::fg::red << "Tests failed :-(" << rang::style::reset
                      << std::endl;
        test_results = !tests_passed;
    }
    else
    {
        std::cout << "# " << rang::style::bold << rang::fg::yellow << "No test has been performed."
                  << rang::style::reset << std::endl;
    }

    // display the statistics of the tasks (if enabled)
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
                const int n_threads = stages[s]->get_n_threads();
                std::cout << "#" << std::endl
                          << "# Pipeline stage " << s << " (" << n_threads << " thread(s)): " << std::endl;
                tools::Stats::show(stages[s]->get_tasks_per_types(), true, false);
            }
        }
    }

    // sockets unbinding
    if (force_sequence)
        sequence_chain->set_n_frames(1);
    else
    {
        pipeline_chain->set_n_frames(1);
        pipeline_chain->unbind_adaptors();
    }

    for (size_t t = 0; t < tsk_types_1d.size() - 1; t++)
    {
        size_t tskid_out = std::get<0>(tsk_2_ids[tsk_types_1d[t].first])[0];
        size_t sckid_out = std::get<2>(tsk_2_ids[tsk_types_1d[t].first])[0];
        size_t tskid_in = std::get<0>(tsk_2_ids[tsk_types_1d[t + 1].first])[0];
        size_t sckid_in = std::get<1>(tsk_2_ids[tsk_types_1d[t + 1].first])[0];

        if (tsk_types_1d[t + 1].first == tsk_e::write && tsk_types_1d[0].first == tsk_e::read)
            tskid_in = std::get<0>(tsk_2_ids[tsk_types_1d[t + 1].first])[1];

        (*modules[t + 1].get())[tskid_in][sckid_in].unbind((*modules[t].get())[tskid_out][sckid_out]);
    }
    if (tsk_types_1d[n_tsk - 1].first == tsk_e::write && tsk_types_1d[0].first == tsk_e::read)
        (*modules[n_tsk - 1].get())[1][1].unbind((*modules[0].get())[0][1]);

#ifdef SPU_HWLOC
    if (!pinning_policy.empty()) tools::Thread_pinning::destroy();
#endif

    return test_results;
}