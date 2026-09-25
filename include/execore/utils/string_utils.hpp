#pragma once

#include <string>
#include <string_view>

namespace execore {

// Decodes escape sequences from a quoted or unquoted string literal
std::string unescape_string(std::string_view raw);

// Decodes a character literal, handling escape sequences
char unescape_char(std::string_view raw);

} // namespace execore
