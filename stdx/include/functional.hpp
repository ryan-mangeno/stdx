#ifndef FUNCTIONAL_HPP
#define FUNCTIONAL_HPP

#include "type_traits.hpp"
#include "alloc.hpp"
#include "exception.hpp"

namespace stdx {

template<typename Signature, unsigned int BufferBytes = 64>
class function;

template<typename R, typename... Args, unsigned int B>
class function<R(Args...), B> {
    struct vtbl_t {
        R (*invoke)(void* obj, Args&&...);
        void (*destroy)(void* obj);
        void (*move)(void* dest, void* src);
    };

public:
    function() noexcept : vptr(nullptr) {}

    explicit operator bool() const noexcept { return vptr != nullptr; }

    R operator()(Args... args) const {
        if (!vptr) throw ::stdx::bad_function_call();
        return vptr->invoke((void*)buffer, ::stdx::forward<Args>(args)...);
    }

    ~function() { reset(); }

    void reset() noexcept {
        if (vptr) {
            vptr->destroy((void*)buffer);
            vptr = nullptr;
        }
    }

    function(function&& other) noexcept : vptr(nullptr) {
        if (other.vptr) {
            other.vptr->move(buffer, other.buffer);
            vptr = other.vptr;
            other.vptr = nullptr;
        }
    }

    function& operator=(function&& other) noexcept {
        if (this != &other) {
            reset();
            if (other.vptr) {
                other.vptr->move(buffer, other.buffer);
                vptr = other.vptr;
                other.vptr = nullptr;
            }
        }
        return *this;
    }

    function(const function&) = delete;
    function& operator=(const function&) = delete;

    template<
      typename F,
      typename = ::stdx::enable_if_t<!::stdx::is_same<::stdx::decay_t<F>, function>::value>
    >
    function(F&& f) {
        using Fun = ::stdx::decay_t<F>;
        // Use the trait to get the Pointer if it's a function, or the Type if it's a Lambda
        using StoredType = typename ::stdx::is_function<Fun>::type;

        static_assert(sizeof(StoredType) <= B, "Callable too large for function buffer");

        new (buffer) StoredType(::stdx::forward<F>(f));

        static constexpr const vtbl_t vt = {
            [](void* obj, Args&&... args) -> R {
                return (*reinterpret_cast<StoredType*>(obj))(::stdx::forward<Args>(args)...);
            },
            [](void* obj) {
                // Pointers have trivial destructors, Lambdas have real ones.
                // StoredType handles both safely.
                reinterpret_cast<StoredType*>(obj)->~StoredType();
            },
            [](void* dest, void* src) {
                new (dest) StoredType(::stdx::move(*reinterpret_cast<StoredType*>(src)));
                reinterpret_cast<StoredType*>(src)->~StoredType();
            }
        };
        vptr = &vt;
    }

private:
    alignas(void*) unsigned char buffer[B];
    const vtbl_t* vptr = nullptr;
};

} // namespace stdx

#endif