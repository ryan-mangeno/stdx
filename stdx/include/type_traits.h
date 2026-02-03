#pragma once

namespace stdx {

    // --- std::remove_reference ---
    template<typename T> struct remove_reference      { using type = T; };
    template<typename T> struct remove_reference<T&>  { using type = T; };
    template<typename T> struct remove_reference<T&&> { using type = T; };
    template<typename T> using  remove_reference_t    = typename remove_reference<T>::type;

    // --- std::move ---
    template<typename T>
    constexpr remove_reference_t<T>&& move(T&& t) noexcept {
        return static_cast<remove_reference_t<T>&&>(t);
    }

    // --- std::forward ---
    template<typename T>
    constexpr T&& forward(remove_reference_t<T>& t) noexcept {
        return static_cast<T&&>(t);
    }

    // --- std::enable_if ---
    template<bool B, typename T = void> struct enable_if {};
    template<typename T>                struct enable_if<true, T> { using type = T; };
    template<bool B, typename T = void> using  enable_if_t = typename enable_if<B, T>::type;

    // --- std::decay (simplified) ---
    template<typename T>
    struct decay {
        using type = remove_reference_t<T>;
    };
    template<typename T> using decay_t = typename decay<T>::type;

    // --- std::is_same ---
    template<typename T, typename U> struct is_same       { static constexpr bool value = false; };
    template<typename T>             struct is_same<T, T> { static constexpr bool value = true; };

    // --- std::is_function ---
    template<typename T> 
    struct is_function { 
        static constexpr bool value = false; 
        using type = T; 
    };

    // This catches "void()" style functions
    template<typename R, typename... A> 
    struct is_function<R(A...)> { 
        static constexpr bool value = true; 
        using type = R(*)(A...); 
    };

    // This catches "void() noexcept" style functions
    template<typename R, typename... A> 
    struct is_function<R(A...) noexcept> { 
        static constexpr bool value = true; 
        using type = R(*)(A...) noexcept; 
    };

}