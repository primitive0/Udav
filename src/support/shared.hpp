#ifndef UDAV_SUPPORT_SHARED_H_
#define UDAV_SUPPORT_SHARED_H_

#include <memory>

template<typename T>
using Shared = std::shared_ptr<T>;

#endif // UDAV_SUPPORT_SHARED_H_
