#pragma once
#include <orc_export.hpp>
#include <stdexcept>

#include <rstring.hpp>

namespace orc::expected {
    template<typename T>
    struct _ok_t {
        T value;
    };
    template<typename E>
    struct _err_t {
        E err;
    };

    template<typename T, typename E>
    class ORC_API expected {
    public:
        expected(const T&) = delete;
        expected(const E&) = delete;
        expected(const expected&) = delete;
        auto operator=(const expected&) = delete;

        expected(T&& val) noexcept : value(std::move(val)) {} // NOLINT
        expected(E&& error) noexcept : err(std::move(error)), state(true) {} // NOLINT
        constexpr expected(_ok_t<T>&& o) noexcept : value(std::move(o.value)) {} // NOLINT
        constexpr expected(_err_t<E>&& e) noexcept : err(std::move(e.err)), state(true) {} // NOLINT

        auto operator=(expected&& other) noexcept -> expected& {
            value = std::move(other.value);
            err = std::move(other.err);
            state = other.state;
            return *this;
        }
        constexpr auto operator=(_ok_t<T>&& o) noexcept -> expected& {
            value = std::move(o.value);
            state = false;
            return *this;
        }
        constexpr auto operator=(_err_t<E>&& e) noexcept -> expected& {
            err = std::move(e.err);
            state = true;
            return *this;
        }

        [[nodiscard]] auto unwrap() const -> T {
            if (state) throw std::runtime_error("cannot unwrap `err` value");
            return std::move(value);
        }
        [[nodiscard]] auto unwrap_or(T&& def) const noexcept -> T {
            if (state) return std::move(def);
            return std::move(value);
        }
        [[nodiscard]] auto expect(const std::string& msg) const -> T {
            if (state) throw std::runtime_error(msg);
            return std::move(value);
        }
        [[nodiscard]] auto expect(const char* msg) const -> T {
            if (state) throw std::runtime_error(msg);
            return std::move(value);
        }
        [[nodiscard]] auto expect(const strings::mutable_u8string<>& msg) const -> T {
            if (state) throw std::runtime_error(static_cast<std::string>(msg));
            return std::move(value);
        }

        [[nodiscard]] auto expect(const std::exception& exc) const -> T {
            if (state) throw exc;
            return std::move(value);
        }

        [[nodiscard]] auto get_err() const -> E&& {
            return std::move(err);
        }

        [[nodiscard]] constexpr auto is_ok() const noexcept -> bool { return state == false; }
        [[nodiscard]] constexpr auto is_err() const noexcept -> bool { return state == true; }

    private:
        T value;
        E err;
        bool state = false;
    };

    template<typename T>
    ORC_API constexpr auto ok(T&& val) noexcept -> _ok_t<T> {
        return { std::forward<T>(val) };
    }
    template<typename E>
    ORC_API constexpr auto err(E&& e) noexcept -> _err_t<E> {
        return { std::forward<E>(e) };
    }
}
