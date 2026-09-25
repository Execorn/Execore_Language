#include "execore/diagnostics/diagnostic_engine.hpp"
#include "execore/frontend/source_manager.hpp"
#include <iostream>

namespace execore {

namespace {
    // ANSI color escape codes
    constexpr const char* COLOR_RESET   = "\033[0m";
    constexpr const char* COLOR_BOLD    = "\033[1m";
    constexpr const char* COLOR_RED     = "\033[1;31m";
    constexpr const char* COLOR_YELLOW  = "\033[1;33m";
    constexpr const char* COLOR_BLUE    = "\033[1;34m";
    constexpr const char* COLOR_CYAN    = "\033[1;36m";
}

DiagnosticEngine::DiagnosticEngine(std::ostream& output_stream, bool use_color)
    : out_(output_stream), use_color_(use_color) {}

void DiagnosticEngine::report(const Diagnostic& diag, const SourceManager* sm) {
    if (diag.level == DiagnosticLevel::Error || diag.level == DiagnosticLevel::Fatal) {
        ++error_count_;
    } else if (diag.level == DiagnosticLevel::Warning) {
        ++warning_count_;
    }
    diagnostics_.push_back(diag);
    render(diag, sm);
}

void DiagnosticEngine::report(DiagnosticLevel level, ErrorCategory cat, const std::string& msg,
                             const SourceSpan& span, const SourceManager* sm) {
    report(Diagnostic{level, cat, msg, span}, sm);
}

void DiagnosticEngine::render(const Diagnostic& diag, const SourceManager* sm) {
    const char* level_color = COLOR_RED;
    const char* level_str = "error";

    switch (diag.level) {
        case DiagnosticLevel::Fatal:
            level_color = COLOR_RED;
            level_str = "fatal error";
            break;
        case DiagnosticLevel::Error:
            level_color = COLOR_RED;
            level_str = "error";
            break;
        case DiagnosticLevel::Warning:
            level_color = COLOR_YELLOW;
            level_str = "warning";
            break;
        case DiagnosticLevel::Note:
            level_color = COLOR_CYAN;
            level_str = "note";
            break;
    }

    // 1. Location prefix: filename:line:col:
    if (!diag.span.filename.empty()) {
        if (use_color_) out_ << COLOR_BOLD;
        out_ << diag.span.filename << ":" << diag.span.start.line << ":" << diag.span.start.column << ": ";
        if (use_color_) out_ << COLOR_RESET;
    }

    // 2. Level and category: [error/SyntaxError]: message
    if (use_color_) out_ << level_color;
    out_ << level_str;
    if (diag.level != DiagnosticLevel::Note) {
        out_ << " [" << to_string(diag.category) << "]";
    }
    out_ << ": ";
    if (use_color_) out_ << COLOR_RESET << COLOR_BOLD;
    out_ << diag.message << "\n";
    if (use_color_) out_ << COLOR_RESET;

    // 3. Source snippet and underline (if source manager available)
    if (sm && !diag.span.filename.empty() && diag.span.start.line > 0) {
        std::string_view line_str = sm->get_line_content(diag.span.filename, diag.span.start.line);
        if (!line_str.empty()) {
            std::string line_num_str = std::to_string(diag.span.start.line);
            std::string indent(line_num_str.size(), ' ');

            // Print source line
            if (use_color_) out_ << COLOR_BLUE;
            out_ << " " << line_num_str << " | ";
            if (use_color_) out_ << COLOR_RESET;
            out_ << line_str << "\n";

            // Print caret / underline pointer
            if (use_color_) out_ << COLOR_BLUE;
            out_ << " " << indent << " | ";
            if (use_color_) out_ << level_color;

            size_t start_col = (diag.span.start.column > 0) ? (diag.span.start.column - 1) : 0;
            size_t span_len = 1;
            if (diag.span.start.line == diag.span.end.line && diag.span.end.column > diag.span.start.column) {
                span_len = diag.span.end.column - diag.span.start.column;
            }

            for (size_t i = 0; i < start_col && i < line_str.size(); ++i) {
                out_ << (line_str[i] == '\t' ? '\t' : ' ');
            }
            out_ << "^";
            for (size_t i = 1; i < span_len; ++i) {
                out_ << "~";
            }
            if (use_color_) out_ << COLOR_RESET;
            out_ << "\n";
        }
    }
}

} // namespace execore
