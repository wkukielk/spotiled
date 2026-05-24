#if !defined(TEST_TIMING_HPP)
#include "test_timing.hpp"
#endif

template <typename Callable>
    requires IsReturnNonVoid<Callable>
auto time_fun(Callable &&func) -> ResultTime<Callable> {
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using hr_clock = std::chrono::high_resolution_clock;

    using ReturnType = std::invoke_result_t<Callable>;
    using DecayedType = std::decay_t<ReturnType>;

    auto start = hr_clock::now();
    auto ret = std::forward<Callable>(func)();
    auto t = hr_clock::now() - start;
    auto ms = duration_cast<milliseconds>(t).count();

    return std::pair<DecayedType, unsigned>{ret, ms};
}

unsigned time_fun(IsReturnVoid auto &&func) {
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using hr_clock = std::chrono::high_resolution_clock;

    auto start = hr_clock::now();
    std::forward<decltype(func)>(func)();
    auto t = hr_clock::now() - start;
    return duration_cast<milliseconds>(t).count();
}

template <typename T> inline auto in_range(T low, T high) {
    using namespace testing;
    return AllOf(Ge(low), Lt(high));
}
