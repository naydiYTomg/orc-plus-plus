#pragma once
#include <ordefs.hpp>
#include <orc_export.hpp>
#include <vector>
using namespace orc::core::defines;

namespace orc::utils::vec {
    template<typename T>
    ORC_API auto extend_to(std::vector<T>& left, std::vector<T>& right, T with) {
        const usize diff = right.size() - left.size();
        for (auto _ = 0; _ < diff; ++_)
            left.push_back(with);
    }
    template<typename T>
    ORC_API auto extend_to(const std::vector<T>& left, const std::vector<T>& right, T with) -> std::vector<T> {
        const usize diff = right.size() - left.size();
        std::vector<T> out;
        for (const auto& item : left)
            out.push_back(item);
        for (auto _ = 0; _ < diff; ++_)
            out.push_back(with);
        return out;
    }
}
