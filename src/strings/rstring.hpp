#pragma once

#include <array>
#include <orc_export.hpp>
#include <ordefs.hpp>
#include <stdexcept>
#include <container.hpp>
#include <memory>
#include <ostream>
#include <string>

using namespace orc::core::defines;

namespace orc::strings {
        using ascii_char = char;

        class ORC_API utf8_char {
        public:
            explicit utf8_char(const ascii_char ascii) : data{static_cast<u8>(ascii), 0, 0, 0}, actual_len(1) {}
            utf8_char(const u8 first, const u8 second, const u8 third, const u8 fourth) : data{first, second, third, fourth}, actual_len(4) {}

            explicit utf8_char(const std::vector<u8>& data) {
                if (data.size() > 4) throw std::invalid_argument("vector size must be <= 4");
                for (usize i = 0; i < data.size(); ++i)
                    this->data[i] = data[i];
                actual_len = data.size();
            }
            ~utf8_char() = default;
            template<usize N>
            requires (N <= 4)
            explicit utf8_char(const std::array<u8, N>& dat) : data{0, 0, 0, 0}, actual_len(N) {
                for (usize i = 0; i < N; ++i) data[i] = dat[i];
            }

            [[nodiscard]] constexpr auto is_ascii() const noexcept -> bool { return actual_len == 1; }
            [[nodiscard]] constexpr explicit operator char() const noexcept { return static_cast<char>(data[0]); }


            friend auto operator<<(std::ostream& os, const utf8_char& ch) -> std::ostream& {
                os.write(reinterpret_cast<const char*>(ch.data.data()), static_cast<std::streamsize>(ch.actual_len));
                return os;
            }

        private:
            std::array<u8, 4> data{};
            usize actual_len;
        };

        template<class Alloc = std::allocator<utf8_char>>
        class ORC_API mutable_u8string final : public core::container::stack_container<utf8_char, Alloc> {
        public:
            mutable_u8string() = default;
            mutable_u8string(const ascii_char* str) {
                const usize len = std::strlen(str);
                reallocate_and_grow(len);
                for (usize i = 0; i < len; ++i)
                    alloc_traits::construct(allocator, data + i, str[i]);
                this->len = len;
            }
            ~mutable_u8string() override {
                destroy_range(data, len);
                deallocate(data, cap);
            }

            mutable_u8string(const mutable_u8string&) = delete;
            auto operator=(const mutable_u8string&) -> mutable_u8string& = delete;

            mutable_u8string(mutable_u8string&&) = default;
            auto operator=(mutable_u8string&&) -> mutable_u8string& = default;

            auto operator=(const ascii_char* str) -> mutable_u8string& {
                const usize len = std::strlen(str);
                if (len > 0) destroy_range(data, this->len);
                if (cap < len) reallocate_and_grow(len);
                for (usize i = 0; i < len; ++i)
                    alloc_traits::construct(allocator, data + i, static_cast<utf8_char>(str[i]));
                this->len = len;
                return *this;
            }

            friend auto operator<<(std::ostream& os, const mutable_u8string& str) -> std::ostream& {
                for (usize i = 0; i < str.len; ++i)
                    os << str.data[i];
                return os;
            }

            [[nodiscard]] constexpr auto size() const noexcept -> usize override { return len; }
            [[nodiscard]] constexpr auto is_empty() const noexcept -> bool override { return len == 0; }
            [[nodiscard]] constexpr auto get(const usize idx) const -> const utf8_char& override {
                if (idx >= len) throw std::out_of_range("index out of range");
                return data[idx];
            }
            [[nodiscard]] constexpr auto get(const usize idx) -> utf8_char& override {
                if (idx >= len) throw std::out_of_range("index out of range");
                return data[idx];
            }
            constexpr auto set(const usize idx, const utf8_char& ch) -> void override {
                if (idx >= len) throw std::out_of_range("index out of range");
                data[idx] = ch;
            }

            [[nodiscard]] constexpr auto reversed() const -> mutable_u8string {
                mutable_u8string result;
                for (isize i = len - 1; i >= 0; --i)
                    result.push(data[i]);
                return result;
            }

            [[nodiscard]] constexpr auto into_reversed() -> mutable_u8string {
                mutable_u8string result;
                for (usize _i = 0; _i < len; ++_i)
                    result.push(pop());
                return result;
            }

            [[nodiscard]] constexpr auto operator[](const usize idx) const -> const utf8_char& override { return get(idx); }
            [[nodiscard]] constexpr auto operator[](const usize idx) -> utf8_char& override { return get(idx); }

            [[nodiscard]] constexpr auto top() -> utf8_char& override { return data[len - 1]; }
            [[nodiscard]] constexpr auto top() const -> const utf8_char& override { return data[len - 1]; }

            [[nodiscard]] constexpr auto is_ascii() const -> bool {
                for (usize i = 0; i < len; ++i)
                    if constexpr (!data[i].is_ascii()) return false;
                return true;
            }

            constexpr auto push(const utf8_char& ch) -> void override {
                if (len == cap)
                    reallocate_and_grow(cap + 1);
                data[len++] = ch;
            }
            constexpr auto push(const char ch) -> void {
                if (len == cap)
                    reallocate_and_grow(cap + 1);
                alloc_traits::construct(allocator, data + len, std::move(ch));
                len++;
            }
            [[nodiscard]] constexpr auto pop() -> utf8_char override {
                if (len == 0) throw std::out_of_range("empty string");
                const utf8_char tmp = data[len - 1];
                alloc_traits::destroy(allocator, &data[len - 1]);
                len--;
                return tmp;
            }

            [[nodiscard]] constexpr explicit operator std::string() const requires(is_ascii()) {
                std::string out;
                for (usize i = 0; i < len; ++i)
                    out.push_back(static_cast<char>(data[i]));
                return out;
            }

        private:
            usize len = 0;
            usize cap = 0;
            Alloc allocator;
            utf8_char* data = nullptr;
            using alloc_traits = std::allocator_traits<Alloc>;

            auto allocate(usize n) -> utf8_char* {
                if (n == 0) return nullptr;
                return alloc_traits::allocate(allocator, n);
            }
            auto deallocate(utf8_char* p, const usize n) -> void {
                if (p) alloc_traits::deallocate(allocator, p, n);
            }
            auto destroy_range(utf8_char* p, const usize count) noexcept -> void {
                for (usize i = 0; i < count; ++i)
                    alloc_traits::destroy(allocator, p + i);
            }
            auto reallocate_and_grow(const usize new_cap) -> void {
                usize target = std::max<usize>(1, cap);
                while (target < new_cap) target *= 2;
                utf8_char* new_data = allocate(target);
                usize moved = 0;
                try {
                    std::uninitialized_move(data, data + len, new_data);
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