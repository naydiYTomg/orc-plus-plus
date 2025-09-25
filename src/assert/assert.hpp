#pragma once
#include <stdexcept>
#include <orc_export.hpp>
namespace orc::assert {
    ORC_API inline auto assert(const bool cond) {
        if (!cond) throw std::runtime_error("assertion failed");
    }

    template<typename T, typename E>
    requires std::equality_comparable_with<T, E>
    ORC_API auto assert_eq(T&& left, E&& right) {
        if (left != right) throw std::runtime_error("assertation failed! left expression != right expression");
    }

    ORC_API inline auto assert(const bool cond, const std::string& msg) {
        if (!cond) throw std::runtime_error(msg);
    }

    template<typename T, typename E>
    requires std::equality_comparable_with<T, E>
    ORC_API auto assert_eq(T&& left, E&& right, const std::string& msg) {
        if (left != right) throw std::runtime_error(msg);
    }
}
