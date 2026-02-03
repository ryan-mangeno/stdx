#pragma once

/**
 * Custom Placement New
 * We must define this globally so the compiler recognizes 'new (ptr) T'.
 * We don't include <new> to avoid MSVC/Apple STL dependencies.
 */

#ifndef STDX_DEBUG // if we are debugging and testing against stl, this will cause issues globally

        inline void* operator new(unsigned long, void* ptr) noexcept { 
            return ptr; 
        }
        // Some compilers also require the matching placement delete
        inline void operator delete(void*, void*) noexcept {}
        
#else
    #include <new>
#endif

#include "type_traits.h"
namespace stdx {

    template<typename T, typename ... Args>
    T* construct_at(void* storage, Args&&... args) noexcept {
        return new (storage) T(::stdx::forward<Args>(args)...);
    }

    template <typename T>
    void delete_at(T* storage) noexcept {
        storage->~T();
    }

}
