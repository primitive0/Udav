module;

#include <exception>

#include "support/string.hpp"

export module udav.runtime.exceptions;

namespace udav {

export class UdavRuntimeException : public std::exception
{
public:
    explicit UdavRuntimeException()
        : message_{}
    {
    }

    explicit UdavRuntimeException(String message)
        : message_{std::move(message)}
    {
    }

    auto what() const noexcept -> const char* override
    {
        return message_.c_str();
    }

private:
    String message_;
};

} // namespace udav
