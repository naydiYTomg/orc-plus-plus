#pragma once
#include <orc_export.hpp>
#include <ordefs.hpp>

using namespace orc::core::defines;

namespace orc::floating {
    struct rfloat {
        unsigned long long sign : 1;
        unsigned long long mantissa : 15;
        unsigned long long num : 48;
    };
}