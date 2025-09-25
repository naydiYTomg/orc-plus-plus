#pragma once
#include <orc_export.hpp>
#include <ordefs.hpp>
#include <type_traits>
#include <stdexcept>

using namespace orc::core::defines;

namespace orc::optional {

    template<typename T>
    struct _some_t {
        T value;
    };
    template<typename... Args>
    struct _some_t_args {
        std::tuple<Args...> args;
        explicit _some_t_args(Args... args) : args(std::forward<Args>(args)...) {}
    };

    struct _some_t_wrapper {
        template<typename T>
        [[nodiscard]] constexpr auto operator()(T&& value) const -> _some_t<T> {
            return _some_t{std::forward<T>(value)};
        }
        template<typename... Args>
        [[nodiscard]] constexpr auto operator()(Args&&... args) const -> _some_t_args<Args...> {
            return _some_t_args{std::forward<Args>(args)...};
        }
    };
    struct _none_t {};

    static constexpr auto some = _some_t_wrapper{};
    static constexpr auto none = _none_t{};


    template<typename T, class Alloc = std::allocator<T>>
    [[deprecated("unstable and unfinished class, do not use")]]
    class ORC_API optional {
    public:
        optional() = default;

        optional(_some_t<T>&& some) {
            storage = alloc_traits::allocate(allocator, 1);
            alloc_traits::construct(allocator, storage, std::move(some.value));
        }
        optional(_none_t&&) noexcept { storage = nullptr; }
        optional(_none_t) noexcept { storage = nullptr; }
        template<typename... Args>
        optional(_some_t_args<Args...>&& arg) {
            storage = alloc_traits::allocate(allocator, 1);
            std::apply([this](auto&&... elems) {
                alloc_traits::construct(allocator, storage, std::forward<decltype(elems)>(elems)...);
            }, std::move(arg.args));
        }

        optional(const optional& other) {
            if (other.storage != nullptr) {
                storage = alloc_traits::allocate(allocator, 1);
                alloc_traits::construct(allocator, storage, *other.storage);
                allocator = other.allocator;
            } else storage = nullptr;
        }
        optional(optional&& other) noexcept {
            storage = other.storage;
            allocator = std::move(other.allocator);
            other.storage = nullptr;
        }

        auto operator=(optional other) noexcept -> optional& {
            swap(*this, other);
            return *this;
        }

        friend auto swap(optional& lhs, optional& rhs) noexcept -> void {
            std::swap(lhs.storage, rhs.storage);
            std::swap(lhs.allocator, rhs.allocator);
        }

        ~optional() {
            if (storage != nullptr) {
                if constexpr (std::is_destructible_v<T>) alloc_traits::destroy(allocator, storage);
                alloc_traits::deallocate(allocator, storage, 1);
            }
        }

        [[nodiscard]] constexpr auto is_some() const noexcept -> bool { return storage != nullptr; }
        [[nodiscard]] constexpr auto is_none() const noexcept -> bool { return storage == nullptr; }
        [[nodiscard]] constexpr auto unwrap_move() -> T {
            if (storage == nullptr) throw std::runtime_error("cannot unwrap `none` value");
            T tmp = std::move(*storage);
            alloc_traits::destroy(allocator, storage);
            alloc_traits::deallocate(allocator, storage, 1);
            storage = nullptr;
            return tmp;
        }
        [[nodiscard]] constexpr auto unwrap() const -> const T& {
            if (storage == nullptr) throw std::runtime_error("cannot unwrap `none` value");
            return *storage;
        }
        [[nodiscard]] constexpr auto unwrap() -> T& {
            if (storage == nullptr) throw std::runtime_error("cannot unwrap `none` value");
            return *storage;
        }
        [[nodiscard]] constexpr auto expect(const std::string& msg) -> T {
            if (storage == nullptr) throw std::runtime_error(msg);
            T tmp = std::move(*storage);
            alloc_traits::destroy(allocator, storage);
            alloc_traits::deallocate(allocator, storage, 1);
            storage = nullptr;
            return tmp;
        }
        using alloc_traits = std::allocator_traits<Alloc>;

    private:
        explicit optional(T&& value) {
            storage = alloc_traits::allocate(allocator, 1);
            alloc_traits::construct(allocator, storage, std::move(value));
        }
        template<typename... Args>
        explicit optional(Args&&... args) {
            storage = alloc_traits::allocate(allocator, 1);
            alloc_traits::construct(allocator, storage, std::forward<Args>(args)...);
        }
        T* storage = nullptr;
        Alloc allocator;
    };



}
