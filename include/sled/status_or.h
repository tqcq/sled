/**
 * @file     : status_or
 * @created  : Thursday Feb 01, 2024 18:04:44 CST
 * @license  : MIT
 **/

#ifndef STATUS_OR_H
#define STATUS_OR_H
#include "sled/optional.h"
#include "sled/status.h"
#include <stdexcept>
#include <type_traits>

namespace sled {

template<typename T>
class StatusOr final {
public:
    static_assert(!std::is_reference<T>::value,
                  "StatusOr<T> requires T to **not** be a reference type");
    using value_type = T;

    StatusOr() : StatusOr(MakeDefaultStatus()) {}

    StatusOr(StatusOr const &) = default;
    StatusOr &operator=(StatusOr const &) = default;

    StatusOr(StatusOr &&other)
        : status_(std::move(other.status_)),
          value_(std::move(value_))
    {
        other.status_ = MakeDefaultStatus();
    }

    StatusOr &operator=(StatusOr &&other)
    {
        status_ = std::move(other.status_);
        value_ = std::move(other.value_);
        other.status_ = MakeDefaultStatus();
        return *this;
    }

    StatusOr(Status rhs) : status_(std::move(rhs))
    {
        if (status_.ok()) {
            throw std::invalid_argument(
                "Status::OK is not a valid argument to StatusOr<T>");
        }
    }

    StatusOr &operator=(Status status)
    {
        *this = StatusOr(std::move(status));
        return *this;
    }

    template<typename U = T,
             /// @code implementation detail
             typename std::enable_if<
                 !std::is_same<StatusOr, typename std::decay<U>::type>::value,
                 int>::type = 0
             /// @code end
             >
    StatusOr &operator=(U &&rhs)
    {
        status_ = Status();
        value_ = std::forward<U>(rhs);
        return *this;
    }

    StatusOr(T &&rhs) : value_(std::move(rhs)) {}

    StatusOr(T const &rhs) : value_(rhs) {}

    bool ok() const { return status_.ok(); }

    explicit operator bool() const { return status_.ok(); }

    T &operator*() & { return *value_; }

    T const &operator*() const & { return *value_; }

    T &&operator*() && { return *std::move(value_); }

    T const &&operator*() const && { return *std::move(value_); }

    T *operator->() & { return &*value_; }

    T const *operator->() const & { return &*value_; }

    T &value() &
    {
        CheckHasValue();
        return **this;
    }

    T const &value() const &
    {
        CheckHasValue();
        return **this;
    }

    T &&value() &&
    {
        CheckHasValue();
        return **this;
    }

    T const &&value() const &&
    {
        CheckHasValue();
        return **this;
    }

    Status const &status() const & { return status_; }

    Status &&status() && { return std::move(status_); }

private:
    static Status MakeDefaultStatus()
    {
        return Status{StatusCode::kUnknown, "default"};
    }

    void CheckHasValue() const &
    {
        if (!ok()) { throw std::invalid_argument("no value"); }
    }

    Status status_;
    sled::optional<T> value_;
};

template<typename T>
bool
operator==(StatusOr<T> const &a, StatusOr<T> const &b)
{
    if (!a || !b) return a.status() == b.status();
}

template<typename T>
bool
operator!=(StatusOr<T> const &a, StatusOr<T> const &b)
{
    return !(a == b);
}

template<typename T>
StatusOr<T>
make_status_or(T rhs)
{
    return StatusOr<T>(std::move(rhs));
}

template<typename T>
StatusOr<T>
make_status_or(StatusCode code, std::string message = "", ErrorInfo info = {})
{
    return StatusOr<T>(Status(code, std::move(message)));
}

}// namespace sled

#endif// STATUS_OR_H
