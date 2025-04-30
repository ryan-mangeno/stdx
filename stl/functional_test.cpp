#include "functional.hpp"
#include <iostream>

int free_function(int x) { return x * 2; }

int main(){
    // storing a free function
    inline_function<int(int), 32> fn = free_function;
    std::cout << fn(21) << "\n"; // 42

    // store a lambda with capture
    long long factor = 3;
    fn = [factor](int x){ int a = 0; return x * factor; };
    std::cout << "Size of fn: " << sizeof(fn) << "\n";  // 32 + 8 for bfufer and vtable

    auto stl_fn = [factor](int x){ return x * factor; };
    std::cout << "Size of stl_fn: " << sizeof(stl_fn) << "\n";

    std::cout << fn(7) << "\n"; // 21

    // move semantics
    inline_function<int(int), 32> fn2 = std::move(fn);
    if (!fn) std::cout << "fn is empty after move\n";
    std::cout << fn2(5) << "\n"; // 15

    return 0;
}