#pragma once

namespace stdx {

    /**
     * @brief Base class for all stdx exceptions.
     * Designed to be logic-only to avoid MSVC/Apple STL dependencies.
     */
    class exception {
    public:
        exception() noexcept = default;
        virtual ~exception() = default;

        // Returns a character string identifying the exception.
        virtual const char* what() const noexcept {
            return "stdx::exception";
        }
    };

    /**
     * @brief Thrown when an invalid stdx::function call is made.
     * Replaces std::bad_function_call.
     */
    class bad_function_call : public exception {
    public:
        bad_function_call() noexcept = default;

        const char* what() const noexcept override {
            return "stdx::bad_function_call: call to empty stdx::function";
        }
    };

} // namespace stdx