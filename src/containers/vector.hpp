#pragma once

#include <orc_export.hpp>
#include <ordefs.hpp>
#include <container.hpp>
#include <memory>
#include <ostream>

#include "rstring.hpp"

using namespace orc::core::container;
using namespace orc::core::defines;

namespace orc::containers {

    template<typename T, class Alloc = std::allocator<T>>
    class ORC_API vector : public stack_container<T, Alloc> {
    public:
        using alloc_traits = std::allocator_traits<Alloc>;
        vector(std::initializer_list<T> init) {
            reallocate_and_grow(init.size()+1);
            for (auto t : init) {
                alloc_traits::construct(allocator, data + len, t);
                len++;
            }
        }
        explicit vector(const usize initial_cap) {
            reallocate_and_grow(initial_cap);
        }
        vector() { reallocate_and_grow(4); }

        vector(const vector& other) {
            if (other.data != nullptr) {
                data = alloc_traits::allocate(allocator, other.cap);
                cap = other.cap;
                for (usize i = 0; i < other.len; i++) {
                    alloc_traits::construct(allocator, data + len, other.data[i]);
                    len++;
                }
                allocator = other.allocator;
            } else data = nullptr;
        }
        vector(vector&& other) noexcept {
            data = other.data;
            cap = other.cap;
            len = other.len;
            allocator = std::move(other.allocator);
            other.data = nullptr;
        }

        ~vector() override {
            if (data != nullptr) {
                destroy_range(data, len);
                deallocate(data, cap);
            }
        }

        [[nodiscard]] constexpr auto size() const noexcept -> usize override { return len; }
        [[nodiscard]] constexpr auto is_empty() const noexcept -> bool override { return len == 0; }
        [[nodiscard]] constexpr auto get(const usize idx) const -> const T& override {
            if (idx >= len) throw std::out_of_range("index out of range");
            return data[idx];
        }
        [[nodiscard]] constexpr auto operator[](const usize idx) const -> const T& override { return get(idx); }

        constexpr auto set(const usize idx, const T& value) -> void override {
            if (idx >= len) throw std::out_of_range("index out of range");
            alloc_traits::destroy(allocator, data + idx);
            alloc_traits::construct(allocator, data + idx, value);
        }
        constexpr auto get(const usize idx) -> T& override {
            if (idx >= len) throw std::out_of_range("index out of range");
            return data[idx];
        }
        constexpr auto operator[](const usize idx) -> T& override { return get(idx); }

        constexpr auto top() const -> const T& override { return get(len - 1); }
        constexpr auto top() -> T& override { return get(0); }

        constexpr auto push(const T& value) -> void override {
            if (len >= cap) reallocate_and_grow(cap+1);
            alloc_traits::construct(allocator, data + len, value);
            len++;
        }
        constexpr auto pop() -> T override {
            if (len == 0) throw std::out_of_range("empty vector");
            const T tmp = data[len - 1];
            alloc_traits::destroy(allocator, data + len - 1);
            len--;
            return tmp;
        }

        friend auto operator<<(std::ostream& os, const vector& vec) -> std::ostream& {
            os << '[';
            for (usize i = 0; i < vec.len; i++) {
                os << vec.get(i);
                if (i != vec.len - 1) os << ", ";
            }
            os << ']';
            return os;
        }

    private:
        T* data = nullptr;
        usize cap = 0;
        usize len = 0;
        Alloc allocator;

        auto allocate(usize n) -> T* {
            if (n == 0) return nullptr;
            return alloc_traits::allocate(allocator, n);
        }
        auto deallocate(T* p, const usize n) -> void {
            if (p) alloc_traits::deallocate(allocator, p, n);
        }
        auto destroy_range(T* p, const usize count) noexcept -> void {
            for (usize i = 0; i < count; ++i)
                alloc_traits::destroy(allocator, p + i);
        }
        auto reallocate_and_grow(const usize new_cap) -> void {
            usize target = std::max<usize>(1, cap);
            while (target < new_cap) target *= 2;
            T* new_data = allocate(target);
            usize moved = 0;
            try {
                if constexpr (std::is_nothrow_move_constructible_v<T>)
                    std::uninitialized_move(data, data + len, new_data);
                else
                    std::uninitialized_copy(data, data + len, new_data);
                moved = len;
            } catch (...) {
                destroy_range(new_data, moved);
                deallocate(new_data, target);
                throw;
            }

            destroy_range(data, len);
            deallocate(data, cap);

            data = new_data;
            cap = target;
        }
    };

}