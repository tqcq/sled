#ifndef SLED_SLED_SYSTEM_PROCESS_H
#define SLED_SLED_SYSTEM_PROCESS_H

#pragma once
#include "sled/optional.h"
#include "sled/synchronization/mutex.h"
#include "sled/system/pid.h"
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace sled {
class Process {
public:
    using ByteReader = std::function<void(const char *, std::size_t)>;

    struct Option {
        std::vector<std::string> cmd_args;
        std::string working_dir;
        std::map<std::string, std::string> envs;
        ByteReader stdout_reader       = nullptr;
        ByteReader stderr_reader       = nullptr;
        bool open_stdin                = false;
        std::size_t stout_buffer_size  = 128 * 1024;
        std::size_t stderr_buffer_size = 128 * 1024;
        bool inherit_file_descriptors  = false;
    };

    struct Builder {
        Builder &SetCmdArgs(const std::vector<std::string> &args);
        Builder &AppendCmdArg(const std::string &arg);
        Builder &SetEnvs(const std::map<std::string, std::string> &env);
        Builder &AppendEnv(const std::string &key, const std::string &value);

        Builder &SetWorkingDir(const std::string &dir);
        Builder &SetStdoutReader(ByteReader reader);
        Builder &SetStderrReader(ByteReader reader);
        Builder &SetOpenStdin(bool open);
        Builder &SetStdoutBufferSize(std::size_t size);
        Builder &SetStderrBufferSize(std::size_t size);
        Builder &SetInheritFileDescriptors(bool inherit);
        Process Build();

    private:
        Option option;
    };

public:
    Process(const Option &option);
    ~Process() noexcept;

    ProcessId id() const noexcept { return id_; }

    sled::optional<int> exit_status() noexcept;
    void CloseStdin() noexcept;
    void Kill(bool force = false) noexcept;
    void Signal(int signal) noexcept;
    std::size_t Write(const char *data, std::size_t size);
    std::size_t Write(const std::string &data);

public:
    static void Kill(ProcessId id, bool force = false) noexcept;
    static void Signal(ProcessId id, int signal) noexcept;

private:
    void CloseAllFileDescriptors() noexcept;

    const Option option;
    ProcessId id_;

    std::unique_ptr<int> stdout_fd_;
    std::unique_ptr<int> stderr_fd_;
    std::unique_ptr<int> stdin_fd_;
    Option option_;
};
}// namespace sled

#endif// SLED_SLED_SYSTEM_PROCESS_H
