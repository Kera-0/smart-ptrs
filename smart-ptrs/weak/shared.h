#pragma once

#include "sw_fwd.h"

#include <cstddef>

template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() : ptr_(nullptr), block_(nullptr) {
    }
    SharedPtr(std::nullptr_t) : ptr_(nullptr), block_(nullptr) {
    }

    explicit SharedPtr(T* ptr) : ptr_(ptr), block_(nullptr) {
        if (ptr_) {
            block_ = new PointingConterBlock<T>(ptr_);
            block_->Increment();
        }
    }
    template <typename U>
    explicit SharedPtr(U* ptr) : ptr_(ptr), block_(nullptr) {
        if (ptr_) {
            block_ = new PointingConterBlock<U>(ptr);
            block_->Increment();
        }
    }

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            block_->Increment();
        }
    }

    SharedPtr(SharedPtr&& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    SharedPtr(T* ptr, ControlBlockBase* block) : ptr_(ptr), block_(block) {
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : ptr_(ptr), block_(other.GetBlock()) {
        if (block_) {
            block_->Increment();
        }
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& other) noexcept : ptr_(other.Get()), block_(other.GetBlock()) {
        other.Set(nullptr);
        other.SetBlock(nullptr);
    }

    void Set(T* ptr) {
        ptr_ = ptr;
    }
    void SetBlock(ControlBlockBase* block) {
        block_ = block;
    }
    explicit SharedPtr(const WeakPtr<T>& other) : ptr_(nullptr), block_(nullptr) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        if (!other.Expired()) {
            block_ = other.GetBlock();
            block_->Increment();
            ptr_ = other.Get();
        } else {

            throw BadWeakPtr();
        }
    }
    template <typename U>
    SharedPtr(const SharedPtr<U>& other) : ptr_(other.Get()), block_(other.GetBlock()) {
        if (block_) {
            block_->Increment();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    template <typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        if (this->Get() != other.Get()) {
            Reset();
            block_ = other.GetBlock();
            ptr_ = other.Get();
            if (block_) {
                block_->Increment();
            }
        }
        return *this;
    }

    template <typename U>
    SharedPtr& operator=(SharedPtr<U>&& other) noexcept {
        if (this->Get() != other.Get()) {
            Reset();
            ptr_ = other.Get();
            block_ = other.GetBlock();
            other.Set(nullptr);
            other.SetBlock(nullptr);
        }
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            Reset();
            block_ = other.block_;
            ptr_ = other.ptr_;
            if (block_) {
                block_->Increment();
            }
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            Reset();
            ptr_ = other.ptr_;
            block_ = other.block_;
            other.ptr_ = nullptr;
            other.block_ = nullptr;
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_) {
            block_->Decrement();
            if (block_->Get1() == 0) {
                block_->Destroy();
                Realise();
            }
            block_ = nullptr;
            ptr_ = nullptr;
        }
    }
    void Realise() const {
        if (block_->Get1() + block_->Get2() == 0) {
            delete block_;
        }
    }

    ControlBlockBase* GetBlock() const {
        return block_;
    }

    void Reset(T* ptr) {
        Reset();
        if (ptr) {
            block_ = new PointingConterBlock<T>(ptr);
            block_->Increment();
        } else {
            block_ = nullptr;
        }
        ptr_ = ptr;
    }

    template <typename U>
    void Reset(U* ptr) {
        Reset();
        if (ptr) {
            block_ = new PointingConterBlock<U>(ptr);
            block_->Increment();
        } else {
            block_ = nullptr;
        }
        ptr_ = ptr;
    }

    void Swap(SharedPtr& other) {
        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        return block_ ? block_->Get1() : 0;
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    T* ptr_;
    ControlBlockBase* block_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    EmplaceConterBlock<T>* block = new EmplaceConterBlock<T>(std::forward<Args>(args)...);
    block->Increment();
    T* ptr = block->Get();
    SharedPtr<T> sp(ptr, block);
    return sp;
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
