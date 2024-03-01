/**
 * @file     : sequence_checker
 * @created  : Saturday Feb 03, 2024 13:32:22 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_SEQUENCE_CHECKER_H
#define SLED_SEQUENCE_CHECKER_H

namespace sled {

class SequenceChecker : public internal::SequenceCheckerImpl {
public:
    enum InitialState : bool {
        kDetached = false,
        kAttached = true,
    };

    explicit SequenceChecker(InitialState initial_state = kAttached) : Impl(initial_state) {}
};

}// namespace sled

#endif// SLED_SEQUENCE_CHECKER_H
