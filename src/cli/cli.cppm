module;

#include <cassert>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <print>
#include <sstream>

#include <lyra/lyra.hpp>

#include "support/string.hpp"
#include "support/unique.hpp"
#include "support/vector.hpp"

export module udav.cli;

import udav.lexer;
import udav.runtime;
import udav.ast;
import udav.parsing;
import udav.interpreter;

namespace udav {

class ExitCliException final : public std::exception
{
public:
    explicit ExitCliException(int exit_code)
        : exit_code_{exit_code}
    {
    }

    auto exit_code() const -> int
    {
        return exit_code_;
    }

private:
    int exit_code_;
};

struct Options final
{
    String filename;
    bool dump_ast;
    bool check_syntax;
};

export class UdavCli final
{
public:
    explicit UdavCli() = default;

    auto run(int argc, char** argv) -> int
    {
        try {
            do_run(argc, argv);
        } catch (const ExitCliException& e) {
            return e.exit_code();
        } catch (const std::exception&) {
            std::cout << "Uncaught std::exception!\n";
            return 1;
        }

        return 0;
    }

private:
    auto do_run(int argc, char** argv) -> void
    {
        parse_arguments(argc, argv);

        if (opts_.check_syntax) {
            check_syntax();
        } else if (opts_.dump_ast) {
            dump_ast();
        } else {
            execute_program();
        }
    }

    auto make_function(
        StrView name,
        ast::Function::NativeCallable callable)
        -> ast::Function
    {
        auto function_node = ast::Function{};
        function_node.name = name;
        function_node.native_callable = std::move(callable);
        return function_node;
    }

    auto execute_program() -> void
    {
        auto source_code = read_program_source();

        try {
            Interpreter{}.execute(source_code);
        } catch (const InterpreterException&) {
            std::cout << "Failed to execute code.\n";
            throw ExitCliException{1};
        }
    }

    auto check_syntax() -> void
    {
        assert(false && "WIP.");
    }

    auto dump_ast() -> void
    {
        auto source_code = read_program_source();

        auto stream = SemanticTokenStream{Lexer{source_code}};
        auto parser = Parser{stream};

        auto program_node = Unique<ast::Program>{};
        try {
            program_node = parser.parse_program();
        } catch (const LexerException&) {
            std::cout << "Failed to lex code.\n";
            throw ExitCliException{1};
        } catch (const ParserException&) {
            std::cout << "Failed to parse code.\n";
            throw ExitCliException{1};
        }

        std::cout << *program_node;
    }

    auto read_program_source() -> String
    {
        if (opts_.filename.empty()) {
            return read_all_stdin();
        }

        auto file = std::ifstream{opts_.filename, std::ios::binary};
        if (!file) {
            std::println("Failed to open file {}.", opts_.filename);
            throw ExitCliException{1};
        }
        return String{std::istreambuf_iterator{file.rdbuf()}, {}};
    }

    auto parse_arguments(int argc, char** argv) -> void
    {
        bool show_help = false;

        auto cli = lyra::cli{}
                       .add_argument(
                           lyra::help{show_help}
                               .description("Interpreter for the Udav programming language."))
                       .add_argument(
                           lyra::opt{opts_.check_syntax}
                               .name("--check-syntax")
                               .help("Only check syntax and exit."))
                       .add_argument(
                           lyra::opt{opts_.dump_ast}
                               .name("--dump-ast")
                               .help("Dump AST and exit."))
                       .add_argument(
                           lyra::arg{opts_.filename, "input-file"}
                               .help("Input source file. If not specified, interpreter"
                                     "will read program from stdin."));

        auto result = cli.parse({argc, argv});
        if (!result) {
            std::println("Error: {}", result.message());
            std::cout << cli;
            throw ExitCliException{1};
        }

        if (show_help) {
            std::cout << cli;
            throw ExitCliException{0};
        }

        if (opts_.check_syntax && opts_.dump_ast) {
            std::println("Error: --check-syntax and --dump-ast cannot be used together.");
            throw ExitCliException{1};
        }
    }

    static auto read_all_stdin() -> String
    {
        auto ss = std::ostringstream{};
        ss << std::cin.rdbuf();
        return ss.str();
    }

    Options opts_{};
};

} // namespace udav
