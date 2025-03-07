#pragma once

#include <exception>

namespace std {
enum class byte : unsigned char;
}
class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

class ControlBlockBase {
public:
    virtual ~ControlBlockBase() = default;

    void Increment() {
        ++count_;
    }
    void Decrement() {
        --count_;
    }
    int Get1() const {
        return count_;
    }
    virtual void Destroy() = 0;

private:
    int count_ = 0;
};

template <typename T>
class PointingConterBlock : public ControlBlockBase {
public:
    explicit PointingConterBlock(T* ptr) : ptr_(ptr) {
    }
    void Destroy() override {
        delete ptr_;
        ptr_ = nullptr;
    }

private:
    T* ptr_;
};

template <typename T>
class EmplaceConterBlock : public ControlBlockBase {
public:
    template <typename... Args>
    explicit EmplaceConterBlock(Args&&... args) {
        new (&buffer_) T(std::forward<Args>(args)...);
    }
    void Destroy() override {
        Get()->~T();
    }
    T* Get() {
        return reinterpret_cast<T*>(&buffer_);
    }

private:
    alignas(T) std::byte buffer_[sizeof(T)];
};
