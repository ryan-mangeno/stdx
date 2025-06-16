#ifndef FUNCTIONAL_HPP
#define FUNCTIONAL_HPP

#include <utility>
#include <type_traits>
#include <iostream>
#include <stdexcept>

#include <functional> 



// explicit small object optimization for std function to avoid heap stuff


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

        // build the vtable for function, since the vtable can be determined at compile time and gets put into static memory since
        // its address is accessed, it also must be static since else it would be a dangling pointer
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
