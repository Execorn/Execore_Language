#pragma once

#include "execore/ast/ast_fwd.hpp"
#include <string>
#include <ostream>

namespace execore {

class DotExporter {
public:
    static void export_to_file(const Program& program, const std::string& filepath);
    static void export_to_stream(const Program& program, std::ostream& out);
};

} // namespace execore
