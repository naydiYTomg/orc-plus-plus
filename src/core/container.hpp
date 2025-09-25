#pragma once
#include <orc_export.hpp>
#include <ordefs.hpp>

using namespace orc::core::defines;

namespace orc::core::container {
    /// immutable container
    template<typename T, class Alloc = std::allocator<T>>
    class ORC_API container {
    public:
        virtual ~container() = default;
        [[nodiscard]] constexpr virtual auto size() const noexcept -> usize = 0;
        [[nodiscard]] constexpr virtual auto is_empty() const noexcept -> bool = 0;
        [[nodiscard]] constexpr virtual auto get(usize) const -> const T& = 0;
        [[nodiscard]] constexpr virtual auto operator[](usize) const -> const T& = 0;
    };

    template<typename T, class Alloc = std::allocator<T>>
    class ORC_API mutable_container : public container<T, Alloc> {
    public:
        ~mutable_container() override = default;
        constexpr virtual auto set(usize, const T&) -> void = 0;
        [[nodiscard]] constexpr virtual auto get(usize) -> T& = 0;
        [[nodiscard]] constexpr virtual auto operator[](usize) -> T& = 0;
    };

    template<typename T, class Alloc = std::allocator<T>>
    class ORC_API top_peekable_container : public container<T, Alloc> {
    public:
        ~top_peekable_container() override = default;
        [[nodiscard]] constexpr virtual auto top() const -> const T& = 0;
    };
    template<typename T, class Alloc = std::allocator<T>>
    class ORC_API mutable_top_peekable_container : public mutable_container<T, Alloc> {
    public:
        ~mutable_top_peekable_container() override = default;
        [[nodiscard]] constexpr virtual auto top() -> T& = 0;
    };

    template<typename T, class Alloc = std::allocator<T>>
    class ORC_API stack_container : public mutable_top_peekable_container<T, Alloc>, public top_peekable_container<T, Alloc> {
    public:
        ~stack_container() override = default;
        constexpr virtual auto push(const T&) -> void = 0;
        [[nodiscard]] constexpr virtual auto pop() -> T = 0;
    };

}