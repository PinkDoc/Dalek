#pragma once

namespace dalek
{
namespace util {
template <typename T, typename U>
struct is_same {
    enum {
        value = false
    }
};

template <typename T>
struct is_same<T, T> {
    enum {
        value = true;
    }
};

} // namespace util
} // namespace dalek
