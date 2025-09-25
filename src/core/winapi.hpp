#pragma once
#include <orc_export.hpp>
#include <ordefs.hpp>
#include <Windows.h>
using namespace orc::core::defines;
namespace orc::core::win {
    ORC_API constexpr u64 WINDOWS_TICKS_PER_SECOND = 10000000;
    ORC_API constexpr i64 EPOCH_DIFF = 11644473600;

    ORC_API inline auto is_key_pressed(const char key) -> bool {
        return GetAsyncKeyState(toupper(key)) & 0x8000 != 0;
    }
    ORC_API constexpr auto windows_ticks_to_unix_seconds(const u64 windows_ticks) -> i64 {
        return static_cast<i64>(windows_ticks / WINDOWS_TICKS_PER_SECOND) - EPOCH_DIFF;
    }
}
