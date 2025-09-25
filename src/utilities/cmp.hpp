#pragma once
#include <orc_export.hpp>
namespace orc::utils::cmp {
    enum class ORC_API ordering {
        Less,
        Greater,
        Equal,
    };

    template<typename T>
    ORC_API constexpr auto cmp(const T& left, const T& right) -> ordering {
        if (left < right) return ordering::Less;
        if (left > right) return ordering::Greater;
        return ordering::Equal;
    }
}