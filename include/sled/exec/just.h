#pragma once
#ifndef SLED_EXEC_JUST_H
#define SLED_EXEC_JUST_H

#include <iostream>
#include <utility>

namespace sled {

struct immovable {
    immovable() = default;
    immovable(immovable &&) = delete;
};

template<typename S, typename R>
using connect_result_t = decltype(connect(std::declval<S>(), std::declval<R>()));

template<typename T>
using sender_result_t = typename T::result_t;

template<typename R, typename T>
struct just_operation : immovable {
    R receiver;
    T value;

    friend void start(just_operation &self) { set_value(self.receiver, self.value); }
};

template<typename T>
struct just_sender {
    using result_t = T;
    T value;

    template<typename R>
    just_operation<R, T> connect(R receiver)
    {
        return {{}, receiver, this->value};
    }
};

template<typename T>
just_sender<T>
just(T t)
{
    return {t};
}

struct cout_receiver {
    template<typename T>
    friend void set_value(cout_receiver self, T &&val)
    {
        std::cout << "Result: " << val << std::endl;
    }

    friend void set_error(cout_receiver self, std::exception_ptr e)
    {
        try {
            std::rethrow_exception(e);
        } catch (const std::exception &e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    friend void set_stopped(cout_receiver self) { std::cout << "Stopped" << std::endl; }
};

}// namespace sled
#endif// SLED_EXEC_JUST_H
