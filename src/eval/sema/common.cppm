module;

#include <exception>

#include "support/string.hpp"

export module udav.eval.sema:common;

namespace udav {

class PassException : public std::exception
{
public:
    explicit PassException()
        : message_{}
    {
    }

    explicit PassException(String message)
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
