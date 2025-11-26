#ifndef UDAV_SUPPORT_UNIQUE_H_
#define UDAV_SUPPORT_UNIQUE_H_

#include <memory>

template<typename T>
using Unique = std::unique_ptr<T>;

#endif // UDAV_SUPPORT_UNIQUE_H_
