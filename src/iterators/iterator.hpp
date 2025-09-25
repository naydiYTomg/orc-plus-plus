#pragma once

#include <orc_export.hpp>
#include <ordefs.hpp>
#include <functional>
#include <container.hpp>
#include <vector.hpp>
#include <utility>
#include <memory>
using namespace orc::core::defines;
using namespace orc::core::container;
using namespace orc::containers;

namespace orc::iterators {

    struct iteration_end final : std::exception{};

    #define foreach(varname, iterable, body)        \
    while (true) {                              \
        try {                                   \
            auto varname = iterable.next();     \
            body                                \
        } catch(iteration_end&) {break;}        \
    }

    template<typename T>
    class ORC_API iterator {
    public:
        using value_type = T;
        virtual ~iterator() = default;
        virtual auto next() -> value_type = 0;
        [[nodiscard]] virtual auto has_next() const noexcept -> bool = 0;
    };

    template<typename T>
    class ORC_API chainable {
    public:
        virtual ~chainable() = default;
        virtual auto collect() -> T = 0;
    };

    template<typename Item, typename Out>
    class ORC_API map_iter final : public chainable<vector<Out>> {
    public:
        map_iter(std::function<Out(Item)> func, std::unique_ptr<iterator<Item>> iter) : action(std::move(func)), obj(std::move(iter)) {}
        auto collect() -> vector<Out> override {
            vector<Out> result;
            foreach(x, (*obj), {
                result.push(action(x));
            })
            return result;;
        }
    private:
        std::function<Out(Item)> action;
        std::unique_ptr<iterator<Item>> obj;
    };

    template<typename T>
    class ORC_API std_vector_iterator final : public iterator<T> {
    public:
        explicit std_vector_iterator(std::vector<T>& vec) {
            begin = vec.begin()._Unwrapped();
            end = vec.end()._Unwrapped();
        }
        [[nodiscard]] constexpr auto has_next() const noexcept -> bool override { return begin+pos != end; }
        [[nodiscard]] constexpr auto next() -> typename iterator<T>::value_type override {
            if (!has_next()) throw iteration_end{};
            auto tmp = *(begin + pos);
            pos++;
            return tmp;
        }
        template<typename Out>
        constexpr auto map(std::function<Out(T)> func) -> map_iter<T, Out> {
            return map_iter<T, Out>(func, std::make_unique<std_vector_iterator>(*this));
        }
    private:
        T* begin = nullptr;
        T* end = nullptr;
        usize pos = 0;
    };
}