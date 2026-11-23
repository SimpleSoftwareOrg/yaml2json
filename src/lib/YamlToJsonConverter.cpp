#include "YamlToJsonConverter.h"
#include "ErrorHandler.h"
#include <ryml_std.hpp>
#include <algorithm>

namespace yaml2json {

namespace {

void clear_container_styles(ryml::Tree& tree, ryml::id_type id) {
    if (!tree.is_container(id)) return;
    tree.set_container_style(id, ryml::NOTYPE);
    for (ryml::id_type ch = tree.first_child(id); ch != ryml::NONE; ch = tree.next_sibling(ch)) {
        clear_container_styles(tree, ch);
    }
}

} // namespace

std::string YamlToJsonConverter::convert(const char* yaml_data, size_t yaml_size) {
    return convert(yaml_data, yaml_size, "");
}

std::string YamlToJsonConverter::convert(const char* yaml_data, size_t yaml_size, const std::string& filename) {
    setup_error_handlers();
    
    try {
        ryml::Tree tree = parse_yaml(yaml_data, yaml_size, filename);
        return tree_to_json(tree);
    } catch (const ConversionError&) {
        throw;
    } catch (const std::exception& e) {
        throw ConversionError("YAML parsing error" + 
                             (filename.empty() ? "" : " in file '" + filename + "'") + 
                             ": " + e.what());
    }
}

ryml::Tree YamlToJsonConverter::parse_yaml(const char* yaml_data, size_t yaml_size, const std::string& filename) {
    // Zero-copy parse with pre-reserved capacity
    size_t est_nodes = std::max<size_t>(1024, yaml_size / 90);
    size_t est_arena = yaml_size * 11 / 10; // 1.1× YAML size

    ryml::Tree tree;
    tree.reserve(est_nodes);
    tree.reserve_arena(est_arena);

    ryml::substr yaml_sub(const_cast<char*>(yaml_data), yaml_size);
    
    if (!filename.empty()) {
        ryml::csubstr file_sub(filename.c_str(), filename.length());
        ryml::parse_in_place(file_sub, yaml_sub, &tree);
    } else {
        ryml::parse_in_place(yaml_sub, &tree);
    }

    // rapidyaml keeps the source's block or flow layout on containers and its JSON emitter
    // follows it; drop it so the output stays on one line, as it always has.
    clear_container_styles(tree, tree.root_id());
    return tree;
}

std::string YamlToJsonConverter::tree_to_json(const ryml::Tree& tree) {
    // A multi-document stream is not one JSON value: refuse it, as rapidyaml did before 0.15.
    const auto opts = ryml::EmitOptions().json_err_on_stream(true);
    return ryml::emitrs_json<std::string>(tree, tree.root_id(), opts);
}

} // namespace yaml2json