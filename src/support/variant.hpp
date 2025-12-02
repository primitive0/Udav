#ifndef UDAV_SUPPORT_VARIANT_H_
#define UDAV_SUPPORT_VARIANT_H_

#include <variant>

template<typename... Ts>
using Variant = std::variant<Ts...>;

#endif // UDAV_SUPPORT_VARIANT_H_
