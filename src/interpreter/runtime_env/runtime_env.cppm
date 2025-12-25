module;

#include <iostream>
#include <utility>

#include "support/string.hpp"
#include "support/vector.hpp"

export module udav.interpreter.runtime_env;

import udav.ast;
import udav.eval;
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
        program.functions.push_back(make_udav_func("str", &to_str));
        program.functions.push_back(make_udav_func("len", &get_length));
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

    static auto to_str(const Vec<UdavValue>& args) -> UdavValue
    {
        if (args.size() != 1) {
            throw EvalException{};
        }

        return args[0].visit(
            [](const UdavString& string) { return UdavValue{UdavString{string}}; },
            [](const UdavInteger& integer) { return UdavValue{UdavString{integer.format()}}; },
            [](const UdavBoolean& boolean) { return UdavValue{UdavString{boolean.format()}}; },
            [](const UdavNull& null) { return UdavValue{UdavString{null.format()}}; });
    }

    static auto get_length(const Vec<UdavValue>& args) -> UdavValue
    {
        if (args.size() != 1) {
            throw EvalException{};
        }

        auto string = args[0].down_cast<UdavString>();
        if (!string) {
            throw EvalException{};
        }
        return UdavValue{UdavInteger{string->size()}};
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
