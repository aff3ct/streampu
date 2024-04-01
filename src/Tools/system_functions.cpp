#if (defined(__GNUC__) || defined(__clang__) || defined(__llvm__)) &&                                                  \
  (defined(__linux__) || defined(__linux) || defined(__APPLE__)) && !defined(__ANDROID__)
#include <cxxabi.h>   // __cxa_demangle
#include <execinfo.h> // backtrace, backtrace_symbols
#endif

#if defined(__linux__) || defined(linux) || defined(__linux) || defined(__FreeBSD__)
#include <errno.h>
#include <string.h>
#include <unistd.h>
#elif defined(__APPLE__) || defined(__MACH__)
#include <mach-o/dyld.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "Tools/Exception/exception.hpp"
#include "Tools/system_functions.h"

std::string
aff3ct::tools::run_system_command(std::string cmd)
{
    std::string data;

    cmd.append(" 2>&1");

#if (defined(__GNUC__) || defined(__clang__) || defined(__llvm__)) &&                                                  \
  (defined(__linux__) || defined(__linux) || defined(__APPLE__)) && !defined(__ANDROID__)
    FILE* stream = popen(cmd.c_str(), "r");
#elif defined(_WIN64) || defined(_WIN32)
    FILE* stream = _popen(cmd.c_str(), "r");
#else
    FILE* stream = nullptr;
#endif

    if (stream)
    {
        const int max_buffer = 256;
        char buffer[max_buffer];
        while (!feof(stream))
            if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);

#if (defined(__GNUC__) || defined(__clang__) || defined(__llvm__)) &&                                                  \
  (defined(__linux__) || defined(__linux) || defined(__APPLE__)) && !defined(__ANDROID__)
        pclose(stream);
#elif defined(_WIN64) || defined(_WIN32)
        _pclose(stream);
#endif
    }
    else
        throw std::runtime_error("run_system_command error: Couldn't open the pipe to run system command.");

    return data;
}

std::string
aff3ct::tools::get_binary_path()
{
    std::string binary_path;

#if defined(__linux__) || defined(linux) || defined(__linux) || defined(__FreeBSD__)
    constexpr size_t path_size = 8192;
    char path[path_size];
#ifdef __FreeBSD__
    auto len = readlink("/proc/curproc/file", path, path_size);
#else
    auto len = readlink("/proc/self/exe", path, path_size);
#endif
    if (len == -1)
    {
        std::stringstream message;
        message << "'readlink' failed ('errno' = " << strerror(errno) << ").";
        throw runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    path[len] = '\0';
    binary_path = path;
#elif defined(__APPLE__) || defined(__MACH__)
    constexpr size_t path_size = 8192;
    char path[path_size];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0)
    {
        std::stringstream message;
        message << "'_NSGetExecutablePath' path buffer is too small; need 'size' = " << size
                << "('path_size' = " << path_size << ").";
        throw runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    binary_path = path;
#elif defined(_WIN32) || defined(_WIN64)
    constexpr size_t path_size = 8192;
    char path[path_size];
    DWORD copied = 0;
    copied = GetModuleFileName(NULL, path, path_size);
    if (copied >= path_size)
    {
        std::stringstream message;
        message << "'GetModuleFileName' path buffer is too small ('path_size' = " << path_size << ").";
        throw runtime_error(__FILE__, __LINE__, __func__, message.str());
    }
    binary_path = path;
#endif

    return binary_path;
}

void
aff3ct::tools::split_path(const std::string& path, std::string& basedir, std::string& filename)
{
    size_t found;
    found = path.find_last_of("/\\");
    basedir = path.substr(0, found);
    filename = path.substr(found + 1);
}