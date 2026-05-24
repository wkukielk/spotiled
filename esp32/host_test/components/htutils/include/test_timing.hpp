#ifndef TEST_TIMING_HPP
#define TEST_TIMING_HPP

#include <chrono>
#include <concepts>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

template <typename Callable>
concept IsReturnVoid =
    std::invocable<Callable> && std::is_void_v<std::invoke_result_t<Callable>>;

template <typename Callable>
concept IsReturnNonVoid = std::invocable<Callable> && !IsReturnVoid<Callable>;

template <typename Callable>
using ResultTime =
    std::pair<std::decay_t<std::invoke_result_t<Callable>>, unsigned>;

template <typename Callable>
    requires IsReturnNonVoid<Callable>
auto time_fun(Callable &&func) -> ResultTime<Callable>;

unsigned time_fun(IsReturnVoid auto &&func);

template <typename T> inline auto in_range(T low, T high);

#include "test_timing.tpp"

#endif