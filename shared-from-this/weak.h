#pragma once
#include "shared.h"

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() : ptr_(nullptr), block_(nullptr) {
    }

    WeakPtr(const WeakPtr& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (!other.Expired()) {
            block_->IncrementWeak();
        }
    }
    WeakPtr(WeakPtr&& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }
    template <typename U>
    WeakPtr(const WeakPtr<U>& other) : ptr_(other.Get()), block_(other.GetBlock()) {
        if (block_) {
            block_->IncrementWeak();
        }
    }
    template <typename U>
    WeakPtr(const SharedPtr<U>& other) : ptr_(other.Get()), block_(other.GetBlock()) {
        if (block_) {
            block_->IncrementWeak();
        }
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        ptr_ = other.Get();
        block_ = other.GetBlock();
        if (block_ != nullptr) {
            block_->IncrementWeak();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) noexcept {
        if (this != &other) {
            if (block_) {
                block_->DecrementWeak();
                Realise();
            }
            ptr_ = other.ptr_;
            block_ = other.block_;
            if (block_) {
                block_->IncrementWeak();
            }
        }
        return *this;
    }
    void Realise() const {
        if (block_->Get1() + block_->Get2() == 0) {
            delete block_;
        }
    }
    WeakPtr& operator=(WeakPtr&& other) noexcept {
        if (this->Get() != other.Get()) {
            Reset();
            ptr_ = other.Get();
            block_ = other.GetBlock();
            other.Setptr();
            other.SetBlock();
        }
        return *this;
    }
    void SetBlock() {
        block_ = nullptr;
    }
    void Setptr() {
        ptr_ = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Reset();
    }
    T* Get() const {
        return ptr_;
    }
    ControlBlockBase* GetBlock() const {
        return block_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (ptr_ != nullptr) {
            block_->DecrementWeak();
            if (block_->Get2() == 0) {
                Realise();
            }
            block_ = nullptr;
            ptr_ = nullptr;
        }
    }
    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        return block_ ? block_->Get1() : 0;
    }
    bool Expired() const {
        if (UseCount() == 0) {
            return true;
        }
        return false;
    }
    SharedPtr<T> Lock() const {
        if (!Expired()) {
            return SharedPtr<T>(*this);
        } else {
            return SharedPtr<T>();
        }
    }

private:
    friend class SharedPtr<T>;
    T* ptr_;
    ControlBlockBase* block_;
};
