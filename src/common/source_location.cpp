#include "execore/common/source_location.hpp"
#include <sstream>

namespace execore {

std::string SourceSpan::to_string() const {
    std::ostringstream oss;
    if (!filename.empty()) {
        oss << filename << ":";
    }
    oss << start.line << ":" << start.column;
    if (start.line != end.line || start.column != end.column) {
        oss << "-" << end.line << ":" << end.column;
    }
    return oss.str();
}

} // namespace execore
