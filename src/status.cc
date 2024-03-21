#include "sled/status.h"
#include <ostream>
#include <string>

namespace sled {

namespace internal {
std::string
StatusCodeToString(StatusCode code)
{
    switch (code) {
    case StatusCode::kOk:
        return "OK";
    case StatusCode::kCancelled:
        return "CANCELLED";
    case StatusCode::kUnknown:
        return "UNKNOWN";
    case StatusCode::kInvalidArgument:
        return "INVALID_ARGUMENT";
    case StatusCode::kDeadlineExceeded:
        return "DEADLINE_EXCEEDED";
    case StatusCode::kNotFound:
        return "NOT_FOUND";
    case StatusCode::kAlreadyExists:
        return "ALREADY_EXISTS";
    case StatusCode::kPermissionDenied:
        return "PERMISSION_DENIED";
    case StatusCode::kUnauthenticated:
        return "UNAUTHENTICATED";
    case StatusCode::kResourceExhausted:
        return "RESOURCE_EXHAUSTED";
    case StatusCode::kFailedPrecondition:
        return "FAILED_PRECONDITION";
    case StatusCode::kAborted:
        return "ABORTED";
    case StatusCode::kOutOfRange:
        return "OUT_OF_RANGE";
    case StatusCode::kUnimplemented:
        return "UNIMPLEMENTED";
    case StatusCode::kInternal:
        return "INTERNAL";
    case StatusCode::kUnavailable:
        return "UNAVAILABLE";
    case StatusCode::kDataLoss:
        return "DATA_LOSS";
    default:
        return "UNEXPECTED_STATUS_CODE=" + std::to_string(static_cast<int>(code));
    }
}
}// namespace internal

std::ostream &
operator<<(std::ostream &os, StatusCode code)
{
    return os << internal::StatusCodeToString(code);
}

bool
operator==(ErrorInfo const &a, ErrorInfo const &b)
{
    return a.reason_ == b.reason_ && a.domain_ == b.domain_ && a.metadata_ == b.metadata_;
}

bool
operator!=(ErrorInfo const &a, ErrorInfo const &b)
{
    return !(a == b);
}

class Status::Impl {
public:
    using PayloadType = std::unordered_map<std::string, std::string>;

    explicit Impl(StatusCode code, std::string message, ErrorInfo error_info, PayloadType payload)
        : code_(code),
          message_(std::move(message)),
          error_info_(std::move(error_info)),
          payload_(std::move(payload))
    {}

    StatusCode code() const { return code_; }

    std::string const &message() const { return message_; }

    ErrorInfo const &error_info() const { return error_info_; }

    PayloadType const &payload() const { return payload_; }

    PayloadType &payload() { return payload_; }

    friend inline bool operator==(Impl const &a, Impl const &b)
    {
        return a.code_ == b.code_ && a.message_ == b.message_ && a.error_info_ == b.error_info_
            && a.payload_ == b.payload_;
    }

    friend inline bool operator!=(Impl const &a, Impl const &b) { return !(a == b); }

private:
    StatusCode code_;
    std::string message_;
    ErrorInfo error_info_;
    PayloadType payload_;
};

Status::Status() = default;
Status::~Status() = default;
Status::Status(Status &&) noexcept = default;

Status &Status::operator=(Status &&) noexcept = default;

// Deep copy
Status::Status(Status const &other) : impl_(other.ok() ? nullptr : new auto(*other.impl_)) {}

// Deep copy
Status &
Status::operator=(Status const &other)
{
    impl_.reset(other.ok() ? nullptr : new auto(*other.impl_));
    return *this;
}

Status::Status(StatusCode code, std::string message, ErrorInfo error_info)
    : impl_(code == StatusCode::kOk ? nullptr : new Status::Impl(code, std::move(message), std::move(error_info), {}))
{}

StatusCode
Status::code() const
{
    return impl_ ? impl_->code() : StatusCode::kOk;
}

std::string const &
Status::message() const
{
    static auto const *const kEmpty = new std::string();
    return impl_ ? impl_->message() : *kEmpty;
}

ErrorInfo const &
Status::error_info() const
{
    static auto const *const kEmpty = new ErrorInfo();
    return impl_ ? impl_->error_info() : *kEmpty;
}

bool
Status::Equals(Status const &a, Status const &b)
{
    return (a.ok() && b.ok()) || (a.impl_ && b.impl_ && *a.impl_ == *b.impl_);
}

std::ostream &
operator<<(std::ostream &os, const Status &s)
{
    if (s.ok()) return os << StatusCode::kOk;
    os << s.code() << ": " << s.message();
    auto const &e = s.error_info();
    if (e.reason().empty() && e.domain().empty() && e.metadata().empty()) { return os; }

    os << "error_info={";
    bool first_field = true;
    if (!e.reason().empty()) {
        os << "reason=" << e.reason();
        first_field = false;
    }
    if (!e.domain().empty()) {
        if (first_field) os << ", ";
        os << ", domain=" << e.domain();
        first_field = false;
    }
    if (!e.metadata().empty()) {
        if (first_field) os << ", ";
        os << "metadata={";
        char const *sep = "";
        for (auto const &item : e.metadata()) {
            os << item.first << "=" << item.second << ", ";
            sep = ", ";
        }
        os << "}";
    }
    return os << "}";
}

namespace internal {
void
AddMetadata(ErrorInfo &error_info, std::string const &key, std::string value)
{
    error_info.metadata_[key] = std::move(value);
}

void
SetPayload(Status &s, std::string key, std::string payload)
{
    if (s.impl_) s.impl_->payload()[std::move(key)] = std::move(payload);
}

}// namespace internal

}// namespace sled
