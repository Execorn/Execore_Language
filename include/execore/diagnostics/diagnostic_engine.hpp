#pragma once

#include "execore/diagnostics/diagnostic.hpp"
#include <vector>
#include <ostream>
#include <functional>

namespace execore {

class SourceManager;

class DiagnosticEngine {
public:
    explicit DiagnosticEngine(std::ostream& output_stream, bool use_color = true);

    void report(const Diagnostic& diag, const SourceManager* sm = nullptr);
    void report(DiagnosticLevel level, ErrorCategory cat, const std::string& msg,
                const SourceSpan& span = {}, const SourceManager* sm = nullptr);

    [[nodiscard]] bool has_errors() const noexcept { return error_count_ > 0; }
    [[nodiscard]] size_t error_count() const noexcept { return error_count_; }
    [[nodiscard]] size_t warning_count() const noexcept { return warning_count_; }
    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept { return diagnostics_; }

    void clear() noexcept {
        diagnostics_.clear();
        error_count_ = 0;
        warning_count_ = 0;
    }

private:
    void render(const Diagnostic& diag, const SourceManager* sm);

    std::ostream& out_;
    bool use_color_{true};
    size_t error_count_{0};
    size_t warning_count_{0};
    std::vector<Diagnostic> diagnostics_;
};

} // namespace execore
