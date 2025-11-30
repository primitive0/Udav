module;

#include "support/unique.hpp"

export module udav.support.convert;

namespace support {

export template<typename T>
auto as_ref(T& value) -> T&
{
    return value;
}

export template<typename T>
auto as_ref(const T& value) -> const T&
{
    return value;
}

export template<typename T>
auto as_ref(Unique<T>& ptr) -> T&
{
    return *ptr;
}

export template<typename T>
auto as_ref(const Unique<T>& ptr) -> const T&
{
    return *ptr;
}

} // namespace support
