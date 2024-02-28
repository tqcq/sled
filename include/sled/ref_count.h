/**
 * @file     : ref_count
 * @created  : Thursday Feb 01, 2024 15:59:19 CST
 * @license  : MIT
 **/

#ifndef REF_COUNT_H
#define REF_COUNT_H

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

#endif// REF_COUNT_H
