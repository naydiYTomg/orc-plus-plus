#pragma once

#include <orc_export.hpp>
#include <ordefs.hpp>
#include <functional>
#include <container.hpp>

#include <utility>
#include <memory>

using namespace orc::core::defines;
using namespace orc::core::container;

namespace orc::iterators {

    struct iteration_end final : std::exception{};

    #define foreach(varname, iterable, body)    \
    while (true) {                              \
        try {                                   \
            auto varname = iterable.next();     \
            body                                \
        } catch(iteration_end&) {break;}        \
    }

    template<typename Item, typename Out, typename Func>
    class ORC_API map_iter;

    template<typename T>
    class ORC_API iterator;

    template<typename T, typename Item>
    concept from_iterator = requires(std::unique_ptr<iterator<Item>> it)
    {
        { T::from_iter(std::move(it)) } -> std::same_as<T>;
    };

    template<typename T>
    class ORC_API iterator {
    public:
        using value_type = T;
        virtual ~iterator() = default;
        [[nodiscard]] virtual auto next() -> value_type = 0;
        [[nodiscard]] virtual auto has_next() const noexcept -> bool = 0;
        virtual auto clone() const -> std::unique_ptr<iterator> = 0;
        template<typename Out, typename Func>
        [[nodiscard]] auto map(Func func) -> std::unique_ptr<iterator> {
            return std::make_unique<map_iter<T, Out, Func>>(std::move(func), this->clone());
        }
        template<from_iterator<T> B>
        [[nodiscard]] auto collect() -> B {
            return B::from_iter(this->clone());
        }
    };




    template<typename Item, typename Out, typename Func>
    class ORC_API map_iter final : public iterator<Out> {
    public:
        map_iter(Func func, std::unique_ptr<iterator<Item>> iter) : action(std::move(func)), obj(std::move(iter)) {}
        auto clone() const -> std::unique_ptr<iterator<Out>> override {
            return std::make_unique<map_iter>(action, obj->clone());
        }
        [[nodiscard]] auto next() -> Out override {
            try { return action(obj->next()); }
            catch (iteration_end&) { throw; }
        }
        [[nodiscard]] auto has_next() const noexcept -> bool override { return obj->has_next(); }
    private:
        Func action;
        std::unique_ptr<iterator<Item>> obj;
    };

    template<typename T>
    class ORC_API std_vector_iterator final : public iterator<T> {
    public:
        explicit std_vector_iterator(std::vector<T>& vec) {
            begin = vec.begin()._Unwrapped();
            end = vec.end()._Unwrapped();
        }
        auto clone() const -> std::unique_ptr<iterator<T>> override {
            return std::make_unique<std_vector_iterator>(*this);
        }
        [[nodiscard]] constexpr auto has_next() const noexcept -> bool override { return begin+pos != end; }
        [[nodiscard]] constexpr auto next() -> typename iterator<T>::value_type override {
            if (!has_next()) throw iteration_end{};
            auto tmp = *(begin + pos);
            pos++;
            return tmp;
        }
    private:
        T* begin = nullptr;
        T* end = nullptr;
        usize pos = 0;
    };





}