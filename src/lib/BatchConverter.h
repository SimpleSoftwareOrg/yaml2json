#pragma once

#include <cstddef>
#include <string>

namespace yaml2json {

// Batch conversion (--batch): every document of a YAML stream becomes one JSON line
//
//   {"file":<name>,"doc":<index>,"duplicate_keys":<bool>,"data":<value>}
//
// The data keeps what YAML 1.2's core schema (and Dart's package:yaml) reads as a boolean
// or a null as a JSON boolean or null; every other scalar becomes a string holding its
// text, so `1.10`, `0x10` and long digit-only checksums keep every character. Aliases are
// expanded; a merge key `<<` stays a plain key. `duplicate_keys` is true when a mapping of
// the document repeats a key. An empty or comment-only stream gives one null document.
class BatchConverter {
public:
    // Appends the lines of every document of `yaml` to `out`. `name` is printed as the
    // "file" field and names the file in errors. Throws ConversionError on invalid YAML
    // or invalid UTF-8; `out` is then left unchanged.
    static void append_documents(std::string& out, const std::string& name,
                                 const char* yaml, size_t size);

    // Returns the lines of every document of `yaml`.
    static std::string convert(const std::string& yaml, const std::string& name);
};

} // namespace yaml2json
