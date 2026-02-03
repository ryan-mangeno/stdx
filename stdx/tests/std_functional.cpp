#include <functional>
#include <chrono>
#include <cstdio>
#include <cstdint>

using clock_sc = std::chrono::steady_clock;
using ns = std::chrono::nanoseconds;

static volatile int sink = 0;

int free_function(int x) { return x + 1; }

template <typename Fn>
void bench(const char* name, Fn& fn, int iterations) {
    auto start = clock_sc::now();
    for (int i = 0; i < iterations; ++i) sink += fn(i);
    auto end = clock_sc::now();

    auto total_ns = std::chrono::duration_cast<ns>(end - start).count();
    double per_call = double(total_ns) / iterations;

    printf("%s: total=%lld ns, per_call=%.2f ns\n", name,
           (long long)total_ns, per_call);
}

int main() {
    constexpr int iters = 200'000'000;
    long long factor = 3;

    // std::function instances (no heap allocation during benchmark)
    std::function<int(int)> std_fn_free = free_function;
    std::function<int(int)> std_fn_small = [factor](int x){ return x * factor; };

    bench("std::free_function", std_fn_free, iters);
    bench("std::small_lambda", std_fn_small, iters);

    return 0;
}
