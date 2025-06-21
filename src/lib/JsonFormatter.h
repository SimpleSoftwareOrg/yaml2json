#pragma once

#include <string>

namespace yaml2json {

// JSON formatting options
struct JsonFormatOptions {
    bool pretty_print = false;
    int indent_size = 2;
    char indent_char = ' ';
    bool add_final_newline = true;
};

// JSON formatter for pretty-printing
class JsonFormatter {
public:
    // Format JSON string according to options
    static std::string format(const std::string& json, const JsonFormatOptions& options = {});
    
    // Pretty-print JSON with default settings (2-space indentation)
    static std::string pretty_print(const std::string& json);
    
    // Compact JSON (remove unnecessary whitespace)
    static std::string compact(const std::string& json);
};

} // namespace yaml2json