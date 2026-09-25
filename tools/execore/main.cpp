#include "execore/frontend/source_manager.hpp"
#include "execore/frontend/lexer.hpp"
#include "execore/frontend/parser.hpp"
#include "execore/semantics/symbol_table.hpp"
#include "execore/semantics/semantic_analyzer.hpp"
#include "execore/runtime/interpreter.hpp"
#include "execore/utils/dot_exporter.hpp"
#include "execore/diagnostics/diagnostic_engine.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace {

void print_help(const char* prog_name) {
    std::cout << "Execore Programming Language - Frontier C++2026 Engine (ISO C++23)\n\n"
              << "Usage:\n"
              << "  " << prog_name << " [options] <source-file>\n\n"
              << "Options:\n"
              << "  -h, --help             Display this help message and exit\n"
              << "  -v, --version          Display version information and exit\n"
              << "  -t, --tab-size <N>     Configure indentation tab size (default: 4)\n"
              << "  --dump-ast             Dump textual representation of AST to stdout\n"
              << "  --emit-dot <file>      Export AST as a Graphviz DOT diagram to <file>\n"
              << "  --check-only           Perform lexical, syntax, and semantic checks without executing\n"
              << "  --no-color             Disable colorized diagnostic messages\n\n"
              << "Examples:\n"
              << "  " << prog_name << " script.exe\n"
              << "  " << prog_name << " --emit-dot ast.dot script.exe\n"
              << "  dot -Tpng ast.dot -o ast.png\n";
}

void print_version() {
    std::cout << "Execore Language Engine version 2.0.0 (ISO C++23 / Frontier C++2026)\n"
              << "Modernized Compiler Pipeline and Interpreted Runtime\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    std::string source_file;
    uint32_t tab_size = 4;
    bool dump_ast = false;
    std::string dot_output_file;
    bool check_only = false;
    bool use_color = true;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_help(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            print_version();
            return 0;
        } else if (arg == "--dump-ast") {
            dump_ast = true;
        } else if (arg == "--emit-dot") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --emit-dot requires an output filename argument\n";
                return 1;
            }
            dot_output_file = argv[++i];
        } else if (arg == "--check-only") {
            check_only = true;
        } else if (arg == "--no-color") {
            use_color = false;
        } else if (arg == "-t" || arg == "--tab-size") {
            if (i + 1 >= argc) {
                std::cerr << "Error: " << arg << " requires a numeric tab size argument\n";
                return 1;
            }
            tab_size = static_cast<uint32_t>(std::stoul(argv[++i]));
        } else if (arg.rfind("-t", 0) == 0 && arg.size() > 2) {
            tab_size = static_cast<uint32_t>(std::stoul(arg.substr(2)));
        } else if (arg.rfind("-D", 0) == 0) {
            // Backward-compatible flag handling: -D4 or -D8 dumped DOT
            if (arg == "-D4" || arg == "-D8") {
                dot_output_file = "ast.dot";
            }
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            print_help(argv[0]);
            return 1;
        } else {
            source_file = arg;
        }
    }

    if (source_file.empty()) {
        std::cerr << "Error: no source file provided.\n";
        print_help(argv[0]);
        return 1;
    }

    execore::SourceManager source_mgr;
    auto source_opt = source_mgr.load_file(source_file);
    if (!source_opt) {
        std::cerr << "Error: could not open source file '" << source_file << "'\n";
        return 1;
    }

    execore::DiagnosticEngine diag(std::cerr, use_color);
    execore::Lexer lexer(*source_opt, source_file, diag, tab_size);
    execore::Parser parser(lexer, diag);

    auto program = parser.parse_program();
    if (diag.has_errors()) {
        return 1;
    }

    // Semantic analysis
    execore::SymbolTable symbols;
    execore::SemanticAnalyzer analyzer(symbols, diag);
    if (!analyzer.analyze(*program)) {
        return 1;
    }

    // Export Graphviz DOT if requested
    if (!dot_output_file.empty()) {
        execore::DotExporter::export_to_file(*program, dot_output_file);
        std::cout << "Generated Graphviz AST: " << dot_output_file << "\n";
    }

    if (dump_ast) {
        execore::DotExporter::export_to_stream(*program, std::cout);
    }

    if (check_only) {
        std::cout << "Check passed: no syntax or semantic errors found.\n";
        return 0;
    }

    // Execution
    execore::Interpreter interpreter(diag, &source_mgr);
    return interpreter.execute(*program);
}
