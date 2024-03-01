/**
 * @file     : sequence_checker_internal
 * @created  : Saturday Feb 03, 2024 13:34:40 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_SYNCHRONIZATION_SEQUENCE_CHECKER_INTERNAL_H
#define SLED_SYNCHRONIZATION_SEQUENCE_CHECKER_INTERNAL_H

#include "sled/synchronization/mutex.h"
#include <string>

namespace sled {

class SequenceCheckerImpl {
public:
    explicit SequenceCheckerImpl(bool attach_to_current_thread);
    ~SequenceCheckerImpl() = default;

    bool IsCurrent() const;
    void Detach();
    std::string ExpectationToString() const;

private:
    mutable Mutex lock_;
    mutable bool attached_;
};

class SequenceCheckerDoNothing {
public:
    explicit SequenceCheckerDoNothing(bool attach_to_current_thread) {}

    bool IsCurrent() const { return true; }

    void Detach() {}
};

template<typename ThreadLikeObject>
typename std::enable_if<std::is_base_of<SequenceCheckerImpl, ThreadLikeObject>::value,
                        std::string>::type
ExpectationToString(const ThreadLikeObject *checker)
{
    return checker->ExpectationToString();
}

template<typename ThreadLikeObject>
typename std::enable_if<!std::is_base_of<SequenceCheckerImpl, ThreadLikeObject>::value,
                        std::string>::type
ExpectationToString(const ThreadLikeObject *checker)
{
    return std::string();
}

}// namespace sled

#endif// SLED_SYNCHRONIZATION_SEQUENCE_CHECKER_INTERNAL_H
