module;

#include <iostream>
#include <utility>

#include "support/string.hpp"
#include "support/vector.hpp"

export module udav.interpreter.runtime_env;

import udav.ast;
import udav.runtime;

namespace udav {

export class RuntimeEnv final
{
public:
    explicit RuntimeEnv() = default;

    auto attach(ast::Program& program) -> void
    {
        program.functions.push_back(make_udav_func("print", &print));
        program.functions.push_back(make_udav_func("println", &println));
    }

private:
    static auto make_udav_func(
        StrView name,
        ast::Function::NativeCallable callable)
        -> ast::Function
    {
        auto function_node = ast::Function{};
        function_node.name = name;
        function_node.native_callable = std::move(callable);
        return function_node;
    }

    static auto print(const Vec<UdavValue>& args) -> UdavValue
    {
        // Forcefully flush stdout, because of specification requirements.
        std::cout << format_args(args) << std::flush;
        return UdavValue{UdavNull{}};
    }

    static auto println(const Vec<UdavValue>& args) -> UdavValue
    {
        std::cout << format_args(args) << std::endl;
        return UdavValue{UdavNull{}};
    }

    static auto format_args(const Vec<UdavValue>& args) -> String
    {
        auto result = String{};
        for (const auto& arg : args) {
            result.append(arg.format());
        }
        return result;
    }
};

} // namespace udav
