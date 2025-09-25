#pragma once
#include <string>
namespace orc::core::concepts {
    template<typename T>
    concept tostring = std::convertible_to<T, std::string> ||
        requires (T a) { { std::to_string(a) } -> std::convertible_to<std::string>; };
}
