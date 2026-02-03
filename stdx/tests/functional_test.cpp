#include "../include/functional.hpp"

#include <stdio.h>

int free_function(int x) { return x * 2; }

void say_hello() {
    printf("Hello\n");
}

struct Greeter {
    void operator()() const {
        printf("Greetings!\n");
    }
};

using namespace stdx;

int main(){

    // polymorphism with function, both greeting functions are type void so this is valid
    function<void(), 32>  greet = say_hello;
    greet();

    greet = Greeter();
    greet();

    printf("Base Size of inline func %zu\n", sizeof(function<void(), 32>)); // 32 bytes for the buffer + vtable pointer

    // storing a free function
    function<int(int), 32> fn = free_function;
    printf("%d\n", fn(21)); // 42
    // store a lambda with capture
    long long factor = 3;
    fn = [factor](int x){ int a = 5; return x * factor; };
    printf("Size of fn: %zu\n", sizeof(fn));  // 32 + 8 for buffer and vtable

    auto stl_fn = [factor](int x){ return x * factor; };
    printf("Size of stl_fn: %zu\n", sizeof(stl_fn));

    printf("%d\n", fn(7)); // 21
    // move semantics
    function<int(int), 32> fn2 = stdx::move(fn);
    if (!fn) printf("fn is empty after move\n");
    printf("%d\n", fn2(5)); // 15

    return 0;
}