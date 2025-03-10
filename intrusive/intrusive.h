#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        ++count_;
        return count_;
    }
    size_t DecRef() {
        --count_;
        return count_;
    }
    size_t RefCount() const {
        return count_;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        if (counter_.DecRef() == 0) {
            Deleter::Destroy(static_cast<Derived*>(this));
        }
    }

    RefCounted& operator=(const RefCounted& other) {
        return *this;
    }

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() : ptr_(nullptr){};
    IntrusivePtr(std::nullptr_t) : ptr_(nullptr){};
    IntrusivePtr(T* ptr) {
        if (ptr != nullptr) {
            ptr_ = ptr;
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) {
        if (other.Get() != nullptr) {
            ptr_ = other.Get();
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    IntrusivePtr(const IntrusivePtr& other) {
        ptr_ = other.Get();
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
    }

    IntrusivePtr(IntrusivePtr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (this != &other) {
            if (other.ptr_ != nullptr) {
                other.ptr_->IncRef();
            }
            if (ptr_ != nullptr) {
                ptr_->DecRef();
            }
            ptr_ = other.ptr_;
        }
        return *this;
    }
    IntrusivePtr& operator=(IntrusivePtr&& other) noexcept {
        if (ptr_ != other.ptr_) {
            if (ptr_ != nullptr) {
                ptr_->DecRef();
            }
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }

        return *this;
    }
    void Set(T* ptr) {
        if (ptr != ptr_) {
            if (ptr != nullptr) {
                ptr->IncRef();
            }
            if (ptr_ != nullptr) {
                ptr_->DecRef();
            }
            ptr_ = ptr;
        }
    }

    // Destructor
    ~IntrusivePtr() {
        Reset();
    }

    // Modifiers
    void Reset() {
        if (ptr_) {
            ptr_->DecRef();
            ptr_ = nullptr;
        }
    }
    void Reset(T* ptr) {
        if (ptr != ptr_) {
            if (ptr != nullptr) {
                ptr->IncRef();
            }
            if (ptr_ != nullptr) {
                ptr_->DecRef();
            }
            ptr_ = ptr;
        }
    }
    void Swap(IntrusivePtr& other) {
        std::swap(ptr_, other.ptr_);
    }

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
        return ptr_ ? ptr_->RefCount() : 0;
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    T* ptr_;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
}
