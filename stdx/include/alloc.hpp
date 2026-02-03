#pragma once

/**
 * Custom Placement New
 * We must define this globally so the compiler recognizes 'new (ptr) T'.
 * We don't include <new> to avoid MSVC/Apple STL dependencies.
 */
inline void* operator new(unsigned long, void* ptr) noexcept { 
    return ptr; 
}

// Some compilers also require the matching placement delete
inline void operator delete(void*, void*) noexcept {}