module;

#include <utility>

export module udav.sema:annotations;

import udav.ast;
import udav.runtime;

namespace udav {

export struct LiteralAnnotation final : public ast::Annotation
{
public:
    UdavValue value;

    explicit LiteralAnnotation(UdavValue value)
        : value{std::move(value)}
    {
    }
};

} // namespace udav
