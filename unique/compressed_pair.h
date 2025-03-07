#pragma once

#include <type_traits>
#include <utility>
#include <vector>

template <typename T, bool Number, bool IsEmpty = std::is_empty_v<T> && !std::is_final_v<T>>
class Element;
template <typename T, bool Number>
class Element<T, Number, true> : private T {
public:
    Element() = default;

    template <typename U>
    explicit Element(U&& value) : T(std::forward<U>(value)) {
    }
    T& Get() {
        return *this;
    }

    const T& Get() const {
        return *this;
    }
};
template <typename T, bool Number>
class Element<T, Number, false> {
public:
    Element() = default;

    template <typename U>
    explicit Element(U&& value) : el_(std::forward<U>(value)) {
    }

    T& Get() {
        return el_;
    }

    const T& Get() const {
        return el_;
    }

private:
    T el_;
};
template <typename F, typename S>
class CompressedPair : private Element<F, false>, private Element<S, true> {
public:
    CompressedPair() : Element<F, false>(), Element<S, true>() {
    }

    template <typename U1, typename U2>
    CompressedPair(U1&& first, U2&& second)
        : Element<F, false>(std::forward<U1>(first)), Element<S, true>(std::forward<U2>(second)) {
    }

    F& GetFirst() {
        return Element<F, false>::Get();
    }

    const F& GetFirst() const {
        return Element<F, false>::Get();
    }

    S& GetSecond() {
        return Element<S, true>::Get();
    }

    const S& GetSecond() const {
        return Element<S, true>::Get();
    }
};
