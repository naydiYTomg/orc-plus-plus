#pragma once
#include <orc_export.hpp>
#include <ordefs.hpp>
#include <iterator.hpp>
#include <vector.hpp>

using namespace orc::core::defines;
using namespace orc::containers;

namespace orc::iterators {
    template<typename T>
    requires (std::is_arithmetic_v<T>)
    class ORC_API infinity_range_iterator final : public iterator<T> {
    public:
        constexpr infinity_range_iterator(T from, T stp) : start(from), step(stp) {}
        constexpr infinity_range_iterator() : start(0), step(1) {}
        explicit constexpr infinity_range_iterator(T from) : start(from), step(1) {}
        [[nodiscard]] constexpr auto has_next() const noexcept -> bool override { return true; }
        [[nodiscard]] constexpr auto next() -> typename iterator<T>::value_type override {
            T tmp = start + step * i;
            i += 1;
            return tmp;
        }
        auto clone() const -> std::unique_ptr<iterator<T>> override {
            return std::make_unique<infinity_range_iterator>(start, step);
        }
        [[nodiscard]] constexpr auto take(const usize n) -> std::shared_ptr<container<T>> {
            vector<T> vec;
            for (usize i = 0; i < n; i++)
                vec.push(start + step * i);
            return std::static_pointer_cast<container<T>, vector<T>>(std::make_shared<vector<T>>(vec));
        }
    private:
        T start;
        T step;
        T i = 0;
    };
}