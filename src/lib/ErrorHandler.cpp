#include "ErrorHandler.h"
#include <sstream>

namespace yaml2json {

void ryml_error_handler(const char* msg, size_t msg_len, ryml::Location loc, void* /*user_data*/) {
    std::string error_msg = "YAML parsing error";
    if (loc.name.not_empty()) {
        error_msg += " in file '";
        error_msg.append(loc.name.str, loc.name.len);
        error_msg += "'";
    }
    error_msg += " at line ";
    error_msg += std::to_string(loc.line);
    error_msg += ", column ";
    error_msg += std::to_string(loc.col);
    error_msg += ": ";
    error_msg.append(msg, msg_len);
    
    throw ConversionError(error_msg);
}

void setup_error_handlers() {
    ryml::set_callbacks(ryml::Callbacks(nullptr, nullptr, nullptr, ryml_error_handler));
}

std::string format_error_with_location(const std::string& base_message, 
                                       const std::string& filename,
                                       size_t line, 
                                       size_t column) {
    std::ostringstream oss;
    oss << base_message;
    if (!filename.empty()) {
        oss << " in file '" << filename << "'";
    }
    if (line > 0) {
        oss << " at line " << line;
        if (column > 0) {
            oss << ", column " << column;
        }
    }
    return oss.str();
}

} // namespace yaml2json