#include "JsonFormatter.h"

namespace yaml2json {

std::string JsonFormatter::format(const std::string& json, const JsonFormatOptions& options) {
    if (!options.pretty_print) {
        return compact(json);
    }
    
    std::string result;
    result.reserve(json.size() * 2);
    
    int indent_level = 0;
    bool in_string = false;
    bool escape_next = false;
    
    for (size_t i = 0; i < json.size(); ++i) {
        char c = json[i];
        
        if (escape_next) {
            result += c;
            escape_next = false;
            continue;
        }
        
        if (c == '\\') {
            result += c;
            escape_next = true;
            continue;
        }
        
        if (c == '"' && !in_string) {
            in_string = true;
            result += c;
            continue;
        } else if (c == '"' && in_string) {
            in_string = false;
            result += c;
            continue;
        }
        
        if (in_string) {
            result += c;
            continue;
        }
        
        switch (c) {
            case '{':
            case '[':
                result += c;
                result += '\n';
                indent_level++;
                for (int j = 0; j < indent_level * options.indent_size; ++j) {
                    result += options.indent_char;
                }
                break;
            case '}':
            case ']':
                if (!result.empty() && result.back() == options.indent_char) {
                    // Remove trailing spaces
                    while (!result.empty() && result.back() == options.indent_char) {
                        result.pop_back();
                    }
                }
                if (!result.empty() && result.back() != '\n') {
                    result += '\n';
                }
                indent_level--;
                for (int j = 0; j < indent_level * options.indent_size; ++j) {
                    result += options.indent_char;
                }
                result += c;
                break;
            case ',':
                result += c;
                result += '\n';
                for (int j = 0; j < indent_level * options.indent_size; ++j) {
                    result += options.indent_char;
                }
                break;
            case ':':
                result += c;
                result += ' ';
                break;
            default:
                if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                    result += c;
                }
                break;
        }
    }
    
    // Add final newline if requested
    if (options.add_final_newline && !result.empty() && result.back() != '\n') {
        result += '\n';
    }
    
    return result;
}

std::string JsonFormatter::pretty_print(const std::string& json) {
    JsonFormatOptions options;
    options.pretty_print = true;
    return format(json, options);
}

std::string JsonFormatter::compact(const std::string& json) {
    // For compact output, the JSON is already compact from rapidyaml
    return json;
}

} // namespace yaml2json