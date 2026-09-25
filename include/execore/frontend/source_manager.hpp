#pragma once

#include "execore/common/source_location.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>

namespace execore {

struct SourceFile {
    std::string filename;
    std::string content;
    std::vector<size_t> line_offsets; // Byte offsets for each line start
};

class SourceManager {
public:
    SourceManager() = default;

    // Load file from disk
    std::optional<std::string_view> load_file(const std::string& filepath);

    // Register file directly from memory
    std::string_view add_source(const std::string& name, std::string content);

    [[nodiscard]] const SourceFile* get_file(std::string_view name) const;
    [[nodiscard]] std::string_view get_source(std::string_view name) const;

    [[nodiscard]] SourceLocation get_location(std::string_view name, size_t offset) const;
    [[nodiscard]] std::string_view get_line_content(std::string_view name, size_t line_number) const;

private:
    void compute_line_offsets(SourceFile& file);

    std::unordered_map<std::string, std::unique_ptr<SourceFile>> files_;
};

} // namespace execore
