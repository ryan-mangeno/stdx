#ifndef FUNCTIONAL_HPP
#define FUNCTIONAL_HPP

#include <utility>
#include <type_traits>
#include <iostream>
#include <stdexcept>

#include <functional> // This seems counterintuitive, but I use std::bad_function_call

/**

note -> unless sbo is used, then this is achiving the same as std::function in that case, but this enforces it

inline_function<R(Args...), N>

This class implements a lightweight, allocation-free function wrapper (similar to std::function) that stores 
callable objects (like lambdas, function pointers, or function objects) directly in a small inline buffer

Key Features:
- **No heap allocations**: Callables are stored inline in a small fixed-size buffer (default: 64 bytes), which eliminates
  the overhead of dynamic memory allocations for small callables.
- **Minimal indirection**: Instead of using a full vtable or control block, the function pointer (`invoke`, `move`, `destroy`)
  is embedded directly into the object, improving performance by reducing memory indirection and improving cache locality
- **Trivially relocatable**: The callables are stored directly in the buffer, which is trivially relocatable and efficient

Benefits:
- **Performance**: No heap allocations, minimal indirections, better cache locality.
- **Small Object Optimization**: If the callable fits within the provided buffer size, everything stays on the stack without
  requiring heap memory, making it efficient for small, frequently invoked callables
- **No heap fragmentation**: Since there's no dynamic memory allocation, this is great for performance-critical applications
  or systems with limited memory like embedded systems.

Drawbacks:
- **Size limitation**: The callable is stored in a fixed-size buffer. If the callable is too large to fit, a compilation error occurs.
- **No polymorphism**: Unlike `std::function`, this class does not support polymorphic callables (i.e., you cannot store multiple
  different types of callable objects unless they are all small enough to fit in the buffer)
- **No dynamic resizing**: If the callable exceeds the buffer size, you must increase the buffer size manually or redesign

Use Cases:
- **High-performance applications**: Where reducing heap allocations and minimizing indirection is crucial for performance
- **Embedded systems**: Where memory constraints require avoiding dynamic allocations and the callables are relatively small


This approach is well-suited to scenarios where you need a fast, lightweight alternative to `std::function` and the callables 
you need to store are small enough to fit within a predefined buffer. It’s not suitable for general-purpose use cases with 
large callables or polymorphic behavior.
 */

template<typename Signature, size_t BufferBytes = 64>
class inline_function;

template<typename R, typename... Args, size_t B>
class inline_function<R(Args...), B> {

public:
    inline_function() noexcept = default;

    // null check
    explicit operator bool() const noexcept { return vptr != nullptr; }

    // Invocation
    R operator()(Args... args) const {
        if (!vptr) throw std::bad_function_call();
        return vptr->invoke((void*)buffer, std::forward<Args>(args)...);
    }

    // Destroy on teardown
    ~inline_function() { reset(); }

    // Reset to empty
    void reset() noexcept {
        if (vptr) {

            /*
    
            Since the object is constructed in-place with placement new, 
            the destructor doesn't automatically get invoked when the object is no longer used
            
            */

            vptr->destroy(buffer);
            vptr = nullptr;
        }
    }

    // Move semantics
    inline_function(inline_function&& other) noexcept {
        if (other.vptr) {
            other.vptr->move(buffer, other.buffer);
            vptr = other.vptr;
            other.vptr = nullptr;
        }
    }
    inline_function& operator=(inline_function&& other) noexcept {
        reset();
        if (other.vptr) {
            other.vptr->move(buffer, other.buffer);
            vptr = other.vptr;
            other.vptr = nullptr;
        }
        return *this;
    }

    // no copy
    inline_function(const inline_function&) = delete;
    inline_function& operator=(const inline_function&) = delete;

    // bind a callable F
    template<
      typename F,
      typename = std::enable_if_t<!std::is_same<std::decay_t<F>, inline_function>::value>
    >
    inline_function(F&& f) {
        using Fun = std::decay_t<F>;
        static_assert(sizeof(Fun) <= B, "Callable too large for inline_function buffer");

        // Placement-new looks like it says “new,” but it doesn’t actually allocate on the heap—it simply 
        // calls the constructor of the functor directly into the pre-allocated buffer memory. 
        // In other words is equivalent to reinterpret_cast<Fun*>(buffer)->Fun(std::forward<F>(f));
        new (buffer) Fun(std::forward<F>(f));

        // build the vtable for Fun (same as before)
        static constexpr const vtbl_t vt = {
            // invoke
            [](void* obj, Args&&... args)->R {
                return (*reinterpret_cast<Fun*>(obj))(std::forward<Args>(args)...);
            },
            // destroy
            [](void* obj) {
                reinterpret_cast<Fun*>(obj)->~Fun();
            },
            // move
            [](void* dest, void* src) {
                new (dest) Fun(std::move(*reinterpret_cast<Fun*>(src)));
                reinterpret_cast<Fun*>(src)->~Fun();
            }
        };
        vptr = &vt;
    }

    private:
        // storage for the callable
        alignas(void*) unsigned char buffer[B];

        // “vtable” of operations (We use inline function pointers instead of an external vtable)
        struct vtbl_t {
            R (*invoke)(void* obj, Args&&...);
            void (*destroy)(void* obj);
            void (*move)(void* dest, void* src);
        };
        const vtbl_t* vptr = nullptr;
};


#endif // FUNCTIONAL_HPP
