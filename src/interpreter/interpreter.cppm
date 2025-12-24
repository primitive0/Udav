module;

#include <exception>

#include "support/string.hpp"
#include "support/unique.hpp"
#include "support/vector.hpp"

export module udav.interpreter;

import udav.lexer;
import udav.runtime;
import udav.ast;
import udav.parsing;
import udav.sema;
import udav.eval;
import udav.interpreter.runtime_env;

namespace udav {

export class InterpreterException final : public std::exception
{
};

export class Interpreter final
{
public:
    explicit Interpreter() = default;

    auto execute(StrView code) -> void
    {
        auto stream = SemanticTokenStream{Lexer{code}};
        auto parser = Parser{stream};

        auto program = Unique<ast::Program>{};
        try {
            program = parser.parse_program();
        } catch (const LexerException&) {
            throw InterpreterException{};
        } catch (const ParserException&) {
            throw InterpreterException{};
        }

        env_.attach(*program);

        try {
            SemanticAnalyzer::run(*program);
        } catch (const PassException& e) {
            throw InterpreterException{};
        }

        // TODO: use constant here instead of main
        auto main_entry = program->function_map.find("main");
        if (main_entry == program->function_map.end()) {
            throw InterpreterException{};
        }
        auto& main_function = *main_entry->second;

        auto value = FunctionEvaluator::eval(*program, main_function, Vec<UdavValue>{});
        if (!value.is_null()) {
            throw InterpreterException{};
        }
    }

private:
    RuntimeEnv env_{};
};

} // namespace udav
