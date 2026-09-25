#include "execore/frontend/source_manager.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace execore {

std::optional<std::string_view> SourceManager::load_file(const std::string& filepath) {
    auto it = files_.find(filepath);
    if (it != files_.end()) {
        return it->second->content;
    }

    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        return std::nullopt;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    // Ensure trailing newline if non-empty
    if (!content.empty() && content.back() != '\n') {
        content.push_back('\n');
    }

    return add_source(filepath, std::move(content));
}

std::string_view SourceManager::add_source(const std::string& name, std::string content) {
    auto file = std::make_unique<SourceFile>();
    file->filename = name;
    file->content = std::move(content);
    compute_line_offsets(*file);

    std::string_view view = file->content;
    files_[name] = std::move(file);
    return view;
}

const SourceFile* SourceManager::get_file(std::string_view name) const {
    auto it = files_.find(std::string(name));
    if (it != files_.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::string_view SourceManager::get_source(std::string_view name) const {
    const auto* file = get_file(name);
    return file ? std::string_view(file->content) : std::string_view{};
}

void SourceManager::compute_line_offsets(SourceFile& file) {
    file.line_offsets.clear();
    file.line_offsets.push_back(0); // Line 1 starts at offset 0

    for (size_t i = 0; i < file.content.size(); ++i) {
        if (file.content[i] == '\n') {
            file.line_offsets.push_back(i + 1);
        }
    }
}

SourceLocation SourceManager::get_location(std::string_view name, size_t offset) const {
    const auto* file = get_file(name);
    if (!file || file->line_offsets.empty()) {
        return SourceLocation{1, 1, static_cast<uint32_t>(offset)};
    }

    // Binary search for line number (1-based)
    auto it = std::upper_bound(file->line_offsets.begin(), file->line_offsets.end(), offset);
    size_t line = static_cast<size_t>(std::distance(file->line_offsets.begin(), it));
    if (line == 0) line = 1;

    size_t line_start = file->line_offsets[line - 1];
    size_t col = (offset >= line_start) ? (offset - line_start + 1) : 1;

    return SourceLocation{
        static_cast<uint32_t>(line),
        static_cast<uint32_t>(col),
        static_cast<uint32_t>(offset)
    };
}

std::string_view SourceManager::get_line_content(std::string_view name, size_t line_number) const {
    const auto* file = get_file(name);
    if (!file || line_number == 0 || line_number > file->line_offsets.size()) {
        return {};
    }

    size_t start = file->line_offsets[line_number - 1];
    size_t end = (line_number < file->line_offsets.size())
                 ? file->line_offsets[line_number]
                 : file->content.size();

    // Strip trailing \r and \n
    while (end > start && (file->content[end - 1] == '\n' || file->content[end - 1] == '\r')) {
        --end;
    }

    return std::string_view(file->content.data() + start, end - start);
}

} // namespace execore
