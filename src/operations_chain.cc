#include "sled/operations_chain.h"
#include "sled/make_ref_counted.h"
#include "sled/optional.h"
#include "sled/scoped_refptr.h"
#include <functional>

namespace sled {
OperationsChain::CallbackHandle::CallbackHandle(scoped_refptr<OperationsChain> operations_chain)
    : operations_chain_(std::move(operations_chain))
{}

OperationsChain::CallbackHandle::~CallbackHandle() {}

void
OperationsChain::CallbackHandle::OnOperationComplete()
{
    has_run_ = true;
    operations_chain_->OnOperationComplete();
    operations_chain_ = nullptr;
}

scoped_refptr<OperationsChain>
OperationsChain::Create()
{
    return scoped_refptr<OperationsChain>(new OperationsChain());
}

OperationsChain::OperationsChain() {}

OperationsChain::~OperationsChain()
{
    // assert IsEmpty()
}

void
OperationsChain::SetOnChainEmptyCallback(std::function<void()> on_chain_empty_callback)
{
    on_chain_empty_callback_ = std::move(on_chain_empty_callback);
}

bool
OperationsChain::IsEmpty() const
{
    return chained_operations_.empty();
}

std::function<void()>
OperationsChain::CreateOpeartionsChainCallback()
{

    auto handle = make_ref_counted<CallbackHandle>(scoped_refptr<OperationsChain>(this));
    return [=] { handle->OnOperationComplete(); };
}

void
OperationsChain::OnOperationComplete()
{
    // assert !empty
    chained_operations_.pop();

    if (!chained_operations_.empty()) {
        chained_operations_.front()->Run();
    } else if (on_chain_empty_callback_.has_value()) {
        on_chain_empty_callback_.value()();
    }
}

}// namespace sled
