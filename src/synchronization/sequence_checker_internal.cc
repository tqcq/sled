#include "sled/synchronization/sequence_checker_internal.h"

namespace sled {
SequenceCheckerImpl::SequenceCheckerImpl(bool attach_to_current_thread)
    : attached_(attach_to_current_thread)
{}

bool
SequenceCheckerImpl::IsCurrent() const
{
    return false;
}
}// namespace sled
