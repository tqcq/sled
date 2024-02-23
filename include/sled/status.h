/**
 * @file     : status
 * @created  : Thursday Feb 01, 2024 17:25:33 CST
 * @license  : MIT
 **/

#ifndef STATUS_H
#define STATUS_H

#include <memory>
#include <string>
#include <unordered_map>

namespace sled {

enum class StatusCode {
    kOk = 0,
    kCancelled = 1,
    kUnknown = 2,
    kInvalidArgument = 3,
    kDeadlineExceeded = 4,
    kNotFound = 5,
    kAlreadyExists = 6,
    kPermissionDenied = 7,
    kResourceExhausted = 8,
    kFailedPrecondition = 9,
    kAborted = 10,
    kOutOfRange = 11,
    kUnimplemented = 12,
    kInternal = 13,
    kUnavailable = 14,
    kDataLoss = 15,
    kUnauthenticated = 16,
};

std::string StatusCodeToString(StatusCode code);
std::ostream &operator<<(std::ostream &os, StatusCode code);

class ErrorInfo;
class Status;

namespace internal {
void AddMetadata(ErrorInfo &, std::string const &key, std::string value);
void SetPayload(Status &s, std::string key, std::string payload);
}// namespace internal

class ErrorInfo {
public:
    ErrorInfo() = default;

    explicit ErrorInfo(std::string reason,
                       std::string domain,
                       std::unordered_map<std::string, std::string> metadata)
        : reason_(std::move(reason)),
          domain_(std::move(domain)),
          metadata_(std::move(metadata))
    {}

    std::string const &reason() const { return reason_; }

    std::string const &domain() const { return domain_; }

    std::unordered_map<std::string, std::string> const &metadata() const
    {
        return metadata_;
    }

    friend bool operator==(ErrorInfo const &, ErrorInfo const &);
    friend bool operator!=(ErrorInfo const &, ErrorInfo const &);

private:
    friend void internal::AddMetadata(ErrorInfo &,
                                      std::string const &key,
                                      std::string value);
    std::string reason_;
    std::string domain_;
    std::unordered_map<std::string, std::string> metadata_;
};

class Status {
public:
    Status();
    ~Status();
    Status(Status const &);
    Status &operator=(Status const &);
    Status(Status &&) noexcept;
    Status &operator=(Status &&) noexcept;

    explicit Status(StatusCode code, std::string message, ErrorInfo info = {});

    bool ok() const { return !impl_; }

    StatusCode code() const;
    std::string const &message() const;
    ErrorInfo const &error_info() const;

    friend bool operator==(Status const &, Status const &);
    friend bool operator!=(Status const &, Status const &);

private:
    friend void internal::SetPayload(Status &, std::string, std::string);
    static bool Equals(Status const &, Status const &);
    class Impl;
    std::unique_ptr<Impl> impl_;
};

std::ostream &operator<<(std::ostream &os, Status const &status);

}// namespace sled

#endif// STATUS_H
