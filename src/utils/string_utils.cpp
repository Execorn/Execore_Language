#include "execore/utils/string_utils.hpp"

namespace execore {

std::string unescape_string(std::string_view raw) {
    // Strip surrounding quotes if present
    if (raw.size() >= 2 && raw.front() == '"' && raw.back() == '"') {
        raw = raw.substr(1, raw.size() - 2);
    }

    std::string result;
    result.reserve(raw.size());

    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\\' && i + 1 < raw.size()) {
            ++i;
            switch (raw[i]) {
                case 'n':  result.push_back('\n'); break;
                case 't':  result.push_back('\t'); break;
                case 'r':  result.push_back('\r'); break;
                case '0':  result.push_back('\0'); break;
                case 'a':  result.push_back('\a'); break;
                case 'b':  result.push_back('\b'); break;
                case 'f':  result.push_back('\f'); break;
                case 'v':  result.push_back('\v'); break;
                case '\\': result.push_back('\\'); break;
                case '"':  result.push_back('"'); break;
                case '\'': result.push_back('\''); break;
                default:
                    result.push_back('\\');
                    result.push_back(raw[i]);
                    break;
            }
        } else {
            result.push_back(raw[i]);
        }
    }
    return result;
}

char unescape_char(std::string_view raw) {
    // Strip surrounding quotes if present
    if (raw.size() >= 2 && raw.front() == '\'' && raw.back() == '\'') {
        raw = raw.substr(1, raw.size() - 2);
    }

    if (raw.empty()) return '\0';

    if (raw.size() >= 2 && raw[0] == '\\') {
        switch (raw[1]) {
            case 'n':  return '\n';
            case 't':  return '\t';
            case 'r':  return '\r';
            case '0':  return '\0';
            case 'a':  return '\a';
            case 'b':  return '\b';
            case 'f':  return '\f';
            case 'v':  return '\v';
            case '\\': return '\\';
            case '\'': return '\'';
            case '"':  return '"';
            default:   return raw[1];
        }
    }

    return raw[0];
}

} // namespace execore
