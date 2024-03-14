#pragma once
#ifndef SLED_FUTURES_TRY_H
#define SLED_FUTURES_TRY_H
#include <exception>
#include <memory>
#include <stdexcept>

namespace sled {

template<typename T>
class Try {
    enum class State {
        kNone,
        kException,
        kValue,
    };

public:
    inline Try() = default;

    Try(const T &t) : state_(State::kValue), value_(t) {}

    Try(T &&t) : state_(State::kValue), value_(std::move(t)) {}

    explicit Try(std::exception_ptr e) : state_(State::kException), exception_(std::move(e)) {}

    Try(Try<T> &&other_try) noexcept : state_(other_try.state_)
    {
        if (state_ == State::Value) {
            new (&value_) T(std::move(other_try.value_));
        } else {
            new (&exception_) std::exception_ptr(std::move(other_try.exception_));
        }
    }

    Try(const Try<T> &other_try) : state_(other_try.state_)
    {
        if (state_ == State::kValue) {
            new (&value_) T(other_try.value_);
        } else {
            new (&exception_) std::exception_ptr(other_try.exception_);
        }
    }

    Try<T> &operator=(Try<T> &&other_try) noexcept
    {
        if (this == &other_try) { return *this; }
        this->~Try();
        state_ = other_try.state_;
        if (state_ == State::kValue) {
            new (&value_) T(std::move(other_try.value_));
        } else {
            new (&exception_) std::exception_ptr(std::move(other_try.exception_));
        }
        return *this;
    }

    Try<T> &operator=(const Try<T> &other_try)
    {
        if (this == &other_try) { return *this; }
        this->~Try();
        state_ = other_try.state_;
        if (state_ == State::kValue) {
            new (&value_) T(other_try.value_);
        } else {
            new (&exception_) std::exception_ptr(other_try.exception_);
        }
        return *this;
    }

    ~Try()
    {
        if (state_ == State::kValue) {
            value_.~T();
        } else if (state_ == State::kException) {
            exception_.~exception_ptr();
        }
    }

    bool HasValue() const noexcept { return state_ == State::kValue; }

    bool HasException() const noexcept { return state_ == State::kException; }

    const T &Value() const &
    {
        AssertHasValue();
        return value_;
    }

    T &Value() &
    {
        AssertHasValue();
        return value_;
    }

    T &&Value() &&
    {
        AssertHasValue();
        return std::move(value_);
    }

    const std::exception_ptr &Exception() const &
    {
        AssertHasException();
        return exception_;
    }

    std::exception_ptr &Exception() &
    {
        AssertHasException();
        return exception_;
    }

    std::exception_ptr &&Exception() &&
    {
        AssertHasException();
        return std::move(exception_);
    }

    const T &operator*() const & { return Value(); }

    T &operator*() & { return Value(); }

    T &&operator*() && { return Value(); }

    explicit operator bool() const { return HasValue(); }

private:
    void AssertHasValue() const
    {
        if (state_ == State::kNone) {
            throw std::runtime_error("Try is empty");
        } else if (state_ == State::kException) {
            std::rethrow_exception(exception_);
        }
    }

    void AssertHasException() const
    {
        if (!HasException()) { throw std::runtime_error("Not exceptioin"); }
    }

    State state_ = State::kNone;

    union {
        T value_;
        std::exception_ptr exception_;
    };
};

template<>
class Try<void> {
    enum class State { kValue, kException };

public:
    Try() : state_(State::kValue) {}

    Try(Try<void> &&other_try) : state_(other_try.state_)
    {
        if (state_ == State::kException) { new (&exception_) std::exception_ptr(std::move(other_try.exception_)); }
    }

    Try(const Try<void> &other_try) : state_(other_try.state_)
    {
        if (state_ == State::kException) { new (&exception_) std::exception_ptr(std::move(other_try.exception_)); }
    }

    explicit Try(std::exception_ptr e) : exception_(std::move(e)) {}

    ~Try()
    {
        if (state_ == State::kException) { exception_.~exception_ptr(); }
    }

    Try<void> &operator=(Try<void> &&other_try)
    {
        if (this == &other_try) { return *this; }
        this->~Try();
        state_ = other_try.state_;
        if (state_ == State::kException) { new (&exception_) std::exception_ptr(std::move(other_try.exception_)); }
        return *this;
    }

    Try<void> &operator=(const Try<void> &other_try)
    {
        if (this == &other_try) { return *this; }
        this->~Try();
        state_ = other_try.state_;
        if (state_ == State::kException) { new (&exception_) std::exception_ptr(std::move(other_try.exception_)); }
        return *this;
    }

    bool HasValue() const noexcept { return state_ == State::kValue; }

    bool HasException() const noexcept { return state_ == State::kException; }

    const std::exception_ptr &Exception() const &
    {
        AssertHasException();
        return exception_;
    }

    std::exception_ptr &Exception() &
    {
        AssertHasException();
        return exception_;
    }

    std::exception_ptr &&Exception() &&
    {
        AssertHasException();
        return std::move(exception_);
    }

    explicit operator bool() const { return HasValue(); }

private:
    void AssertHasException() const
    {
        if (!HasException()) { throw std::runtime_error("Not exception"); }
    }

    void AssertHasValue() const
    {
        if (!HasValue()) { throw std::runtime_error("Try is empty"); }
    }

    State state_;
    std::exception_ptr exception_;
};

template<typename T>
struct TryWrapper {
    using Type = Try<T>;
};

template<typename T>
struct TryWrapper<Try<T>> {
    using Type = Try<T>;
};

template<typename F, typename... Args, typename ResultT = typename std::result_of<F(Args...)>::value>
typename std::enable_if<!std::is_same<void, ResultT>::value, typename TryWrapper<ResultT>::Type>::type
WrapWithTry(F &&f, Args &&...args)
{
    try {
        return typename TryWrapper<ResultT>::Type(std::forward<F>(f)(std::forward<Args>(args)...));
    } catch (...) {
        return typename TryWrapper<ResultT>::Type(std::current_exception());
    }
}

template<typename F, typename... Args, typename ResultT = typename std::result_of<F(Args...)>::value>
typename std::enable_if<std::is_same<void, ResultT>::value, typename TryWrapper<ResultT>::Type>::type
WrapWithTry(F &&f, Args &&...args)
{
    try {

        std::forward<F>(f)(std::forward<Args>(args)...);
        return Try<void>();
    } catch (...) {
        return Try<void>(std::current_exception());
    }
}

}// namespace sled
#endif// SLED_FUTURES_TRY_H
