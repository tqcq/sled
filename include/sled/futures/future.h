#ifndef SLED_FUTURES_FUTHRE_H
#define SLED_FUTURES_FUTHRE_H

#include "sled/any.h"
#include "sled/exec/detail/invoke_result.h"
#include "sled/optional.h"
#include "sled/synchronization/mutex.h"
#include <exception>
#include <memory>

namespace sled {

template<typename T>
class Future;
template<typename T>
class Promise;

template<typename T>
struct FPState : std::enable_shared_from_this<FPState<T>> {
    sled::Mutex lock;
    sled::optional<T> data;
    std::exception_ptr err;
    bool done;
    sled::any priv;
};

template<typename T>
class Future {
public:
    using result_t = T;

    Future(std::shared_ptr<FPState<T>> state) : state_(state) {}

private:
    std::shared_ptr<FPState<T>> state_;
};

template<typename T>
class Promise {
public:
    using result_t = T;

    void SetValue(T &&value) {}

    void SetError(std::exception_ptr err) {}

    Future<T> GetFuture() {}
};

}// namespace sled
#endif//  SLED_FUTURES_FUTHRE_H
