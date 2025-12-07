// Explanation:
// https://github.com/CLIUtils/CLI11/tree/f10ee369ee44f7ecb631e452456c8e61dda734b3#validators-that-may-be-disabled-
#define CLI11_ENABLE_EXTRA_VALIDATORS 1

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ostream>
#include <print>

#include <CLI/CLI.hpp>

#include "support/numerics.hpp"
#include "support/option.hpp"
#include "support/string.hpp"
#include "support/tree_map.hpp"
#include "support/unique.hpp"

import udav.lexer;
import udav.ast;
import udav.parsing;

namespace udav {

enum class UtilityRoutine {
    DumpAst,
};

static const TreeMap<String, UtilityRoutine> kToUtilityRoutine{
    {"dump_ast", UtilityRoutine::DumpAst},
};

auto operator<<(std::ostream& os, const UtilityRoutine& value) -> std::ostream&
{
    using enum UtilityRoutine;

    switch (value) {
    case DumpAst:
        return os << "dump_ast";
    default:
        assert(false && "Switch is not exhaustive.");
    }
}

struct Options final
{
    String filename{};
    Option<UtilityRoutine> utility_routine{};
};

auto must_parse_command_line(int argc, char** argv) -> Options
{
    auto opts = Options{};

    auto app = CLI::App{"Udav interpreter"};

    app.add_option("file", opts.filename, "Udav source file")
        ->required();

    app.add_option("-U,--utility", opts.utility_routine,
           "Run utility routine instead of executing program "
           "(routines: dump_ast)")
        ->transform(CLI::CheckedTransformer(kToUtilityRoutine));

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        std::exit(app.exit(e)); // NOLINT(concurrency-mt-unsafe)
    }

    return opts;
}

// TODO: better run functions (add exceptions).
// TODO: move all this functions in one class

auto dump_ast(const Options& opts) -> i32
{
    auto input = std::ifstream{opts.filename, std::ios::binary};
    if (!input) {
        std::println("Failed to open file {}.", opts.filename);
        return 1;
    }
    auto source_text = String{std::istreambuf_iterator{input.rdbuf()}, {}};
    if (!input) {
        std::println("Error happened while reading file {}.", opts.filename);
        return 1;
    }

    auto stream = SemanticTokenStream{Lexer{source_text}};
    auto parser = Parser{stream};

    auto program_node = Unique<ast::Program>{};
    try {
        program_node = parser.parse_program();
    } catch (const LexerException&) {
        std::cout << "Failed to lex code.\n";
    } catch (const ParserException&) {
        std::cout << "Failed to parse code.\n";
    }

    std::cout << *program_node;

    return 0;
}

auto run_utility_routine(const Options& opts, UtilityRoutine routine) -> i32
{
    switch (routine) {
    case UtilityRoutine::DumpAst:
        return dump_ast(opts);
    default:
        assert(false && "Switch is not exhaustive.");
    }
}

auto run(const Options& opts) -> i32
{
    if (opts.utility_routine) {
        return run_utility_routine(opts, *opts.utility_routine);
    }

    std::println("Interpretation is WIP.");
    return 1;
}

} // namespace udav

auto main(int argc, char** argv) -> int
{
    auto opts = udav::must_parse_command_line(argc, argv);
    return static_cast<int>(udav::run(opts));
}
