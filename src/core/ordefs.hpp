#pragma once
#include <vector>
#define repeat(n, body) \
    for (usize _ = 0; _ < n; _++) body

namespace orc::core::defines {
    using u8 = unsigned char;
    using u16 = unsigned short;
    using u32 = unsigned int;
    using u64 = unsigned long long;
    using i8 = char;
    using i16 = short;
    using i32 = int;
    using i64 = long long;
    using usize = unsigned long long;
    using isize = long long;
}