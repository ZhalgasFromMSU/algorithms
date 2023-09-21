#pragma once

#include <type_traits>
#include <variant>
#include <system_error>

namespace algo {

    template<typename T, typename E = std::error_condition>
        requires (
            !std::is_reference_v<T>
            && !std::is_same_v<std::remove_cv_t<T>, void>
            && !std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<E>>
        )
    class Expected : public std::variant<T, E> {
        using Base = std::variant<T, E>;

    public:
        Expected(E ec)
            : Base{std::move(ec)}
        {}

        template<typename... Args>
        Expected(Args&&... args)
            : Base{std::in_place_type<T>, std::forward<Args>(args)...}
        {}

        operator bool() const noexcept {
            return !std::holds_alternative<E>(*this);
        }

        T&& operator*() && noexcept {
            return std::get<T>(std::move(*this));
        }

        T& operator*() & noexcept {
            return std::get<T>(*this);
        }

        const T& operator*() const & noexcept {
            return std::get<T>(*this);
        }

        T* operator->() noexcept {
            return &std::get<T>(*this);
        }

        const T* operator->() const noexcept {
            return &std::get<T>(*this);
        }

        const E& Error() const noexcept {
            if (*this) {
                return E{};
            }

            return std::get<E>(*this);
        }

    };

} // namespace algo