/**
 * @file     : ref_count
 * @created  : Thursday Feb 01, 2024 15:59:19 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_REF_COUNT_H
#define SLED_REF_COUNT_H

namespace sled {

enum class RefCountReleaseStatus { kDroppedLastRef, kOtherRefsRemained };

class RefCountInterface {
public:
    virtual void AddRef() const = 0;
    virtual RefCountReleaseStatus Release() const = 0;

protected:
    virtual ~RefCountInterface() = default;
};

}// namespace sled

#endif// SLED_REF_COUNT_H
