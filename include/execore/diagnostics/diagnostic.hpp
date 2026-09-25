#pragma once

#include "execore/common/source_location.hpp"
#include "execore/common/error_code.hpp"
#include <string>

namespace execore {

enum class DiagnosticLevel : uint8_t {
    Note,
    Warning,
    Error,
    Fatal
};

struct Diagnostic {
    DiagnosticLevel level{DiagnosticLevel::Error};
    ErrorCategory category{ErrorCategory::Syntax};
    std::string message;
    SourceSpan span;

    static Diagnostic error(ErrorCategory cat, std::string msg, SourceSpan sp = {}) {
        return Diagnostic{DiagnosticLevel::Error, cat, std::move(msg), std::move(sp)};
    }

    static Diagnostic warning(ErrorCategory cat, std::string msg, SourceSpan sp = {}) {
        return Diagnostic{DiagnosticLevel::Warning, cat, std::move(msg), std::move(sp)};
    }

    static Diagnostic note(std::string msg, SourceSpan sp = {}) {
        return Diagnostic{DiagnosticLevel::Note, ErrorCategory::Runtime, std::move(msg), std::move(sp)};
    }
};

} // namespace execore
