#pragma once

#ifdef _WIN32
#ifdef ORC_EXPORT
#define ORC_API __declspec(dllexport)
#else
#define ORC_API __declspec(dllimport)
#endif
#else
#define ORC_API
#endif
