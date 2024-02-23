/**
 * @file     : operations_chain
 * @created  : Saturday Feb 03, 2024 12:40:36 CST
 * @license  : MIT
 **/

#ifndef OPERATIONS_CHAIN_H
#define OPERATIONS_CHAIN_H

#include "sled/optional.h"
#include "sled/ref_counted_base.h"
#include "sled/scoped_refptr.h"
#include <functional>
#include <memory>
#include <queue>

namespace sled {

namespace internal {
class Operation {
public:
    virtual ~Operation() {}

    virtual void Run() = 0;
};

template<typename FunctorT>
class OperationWithFunctor final : public Operation {
public:
    OperationWithFunctor(FunctorT &&functor, std::function<void()> callback)
        : functor_(std::forward<FunctorT>(functor)),
          callback_(std::move(callback))
    {}

    ~OperationWithFunctor() override {}

    void Run() override
    {
        has_run_ = true;
        auto functor = std::move(functor_);
        functor(std::move(callback_));
    }

private:
    typename std::remove_reference<FunctorT>::type functor_;
    std::function<void()> callback_;
    bool has_run_ = false;
};
}// namespace internal

class OperationsChain final : public RefCountedNonVirtual<OperationsChain> {
public:
    static scoped_refptr<OperationsChain> Create();
    ~OperationsChain();
    OperationsChain(const OperationsChain &) = delete;
    OperationsChain operator=(const OperationsChain &) = delete;

    void SetOnChainEmptyCallback(std::function<void()> on_chain_empty_callback);
    bool IsEmpty() const;

    template<typename FunctorT>
    void ChainOperation(FunctorT &&functor)
    {
        auto wrapper = new internal::OperationWithFunctor<FunctorT>(std::forward<FunctorT>(functor), CreateOpeartionsChainCallback());
        chained_operations_.push(std::unique_ptr<internal::OperationWithFunctor<FunctorT>>(wrapper));

        if (chained_operations_.size() == 1) { chained_operations_.front()->Run(); }
    }

private:
    friend class CallbackHandle;

    class CallbackHandle final : public RefCountedNonVirtual<CallbackHandle> {
    public:
        explicit CallbackHandle(scoped_refptr<OperationsChain> operations_chain);
        ~CallbackHandle();

        CallbackHandle(const CallbackHandle &) = delete;
        CallbackHandle &operator=(const CallbackHandle &) = delete;
        void OnOperationComplete();

    private:
        scoped_refptr<OperationsChain> operations_chain_;
        bool has_run_;
    };

    OperationsChain();
    std::function<void()> CreateOpeartionsChainCallback();
    void OnOperationComplete();

    std::queue<std::unique_ptr<internal::Operation>> chained_operations_;
    sled::optional<std::function<void()>> on_chain_empty_callback_;
};

}// namespace sled

#endif// OPERATIONS_CHAIN_H
