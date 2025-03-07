#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <type_traits>
#include <utility>
template <typename T>
struct DefaultDelete {
    DefaultDelete() = default;
    ~DefaultDelete() = default;
    template <typename T1>
    DefaultDelete(T1&& other) noexcept {
    }
    template <typename T1>
    DefaultDelete& operator=(T1&& other) noexcept {
        return *this;
    }
    void operator()(T* p) const {
        static_assert(!std::is_void_v<T>);
        static_assert(sizeof(T) > 0);
        delete p;
    }
};
template <typename T>
struct DefaultDelete<T[]> {
    DefaultDelete() = default;
    ~DefaultDelete() = default;
    template <typename T1>
    DefaultDelete(T1&& other) noexcept {
    }
    template <typename T1>
    DefaultDelete& operator=(T1&& other) noexcept {
        return *this;
    }
    void operator()(T* p) const {
        static_assert(!std::is_void_v<T>);
        static_assert(sizeof(T) > 0);
        delete[] p;
    }
};
// Primary template
template <typename T, typename Deleter = DefaultDelete<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : pair_(ptr, Deleter()) {
    }

    UniquePtr(T* ptr, Deleter deleter) noexcept : pair_(ptr, std::move(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept : pair_(other.Release(), std::move(other.GetDeleter())) {
    }
    template <typename U, typename E>
    UniquePtr(UniquePtr<U, E>&& other) noexcept
        : pair_(other.Release(), std::move(other.GetDeleter())) {
    }
    UniquePtr(UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    UniquePtr& operator=(const UniquePtr& other) = delete;
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            Reset();
            pair_.GetFirst() = other.Release();
            pair_.GetSecond() = std::move(other.GetDeleter());
        }
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset(nullptr);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* ptr = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return ptr;
    }
    void Reset(T* ptr = nullptr) {
        auto ptr2 = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        Deleter()(ptr2);
    }
    void Swap(UniquePtr& other) {
        std::swap(pair_.GetFirst(), other.pair_.GetFirst());
        std::swap(pair_.GetSecond(), other.pair_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    }
    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }
    explicit operator bool() const {
        return pair_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *pair_.GetFirst();
    }
    T* operator->() const {
        return pair_.GetFirst();
    }

private:
    T* Getponinter() {
        return pair_.GetFirst();
    }

    CompressedPair<T*, Deleter> pair_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    explicit UniquePtr(T* ptr = nullptr) noexcept : pair_(ptr, Deleter()) {
    }

    UniquePtr(T* ptr, Deleter deleter) noexcept : pair_(ptr, std::move(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept : pair_(other.Release(), std::move(other.GetDeleter())) {
    }
    template <typename U, typename E>
    UniquePtr(UniquePtr<U, E>&& other) noexcept
        : pair_(other.Release(), std::move(other.GetDeleter())) {
    }
    UniquePtr(UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    UniquePtr& operator=(const UniquePtr& other) = delete;
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            Reset();
            pair_.GetFirst() = other.Release();
            pair_.GetSecond() = std::move(other.GetDeleter());
        }
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset(nullptr);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* ptr = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return ptr;
    }
    void Reset(T* ptr = nullptr) {
        auto ptr2 = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        Deleter()(ptr2);
    }
    void Swap(UniquePtr& other) {
        std::swap(pair_.GetFirst(), other.pair_.GetFirst());
        std::swap(pair_.GetSecond(), other.pair_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    }
    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }
    explicit operator bool() const {
        return pair_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *pair_.GetFirst();
    }
    T* operator->() const {
        return pair_.GetFirst();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator[](size_t index) {
        return (Get())[index];
    }
    const T& operator[](size_t index) const {
        return (Get())[index];
    }

private:
    T* Getponinter() {
        return pair_.GetFirst();
    }
    CompressedPair<T*, Deleter> pair_;
};
