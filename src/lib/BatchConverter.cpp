#include "BatchConverter.h"
#include "ErrorHandler.h"

#include <algorithm>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace yaml2json {

namespace {

using ryml::csubstr;
using ryml::id_type;
using ryml::NodeType;

std::string_view view(csubstr s) { return std::string_view(s.str, s.len); }

// YAML 1.2 core schema, as package:yaml reads it.
bool is_null_text(std::string_view s) {
    return s.empty() || s == "~" || s == "null" || s == "Null" || s == "NULL";
}

// 1 for true, 0 for false, -1 for anything else.
int bool_value(std::string_view s) {
    if (s == "true" || s == "True" || s == "TRUE") return 1;
    if (s == "false" || s == "False" || s == "FALSE") return 0;
    return -1;
}

void append_json_string(std::string& out, std::string_view s) {
    static const char hex[] = "0123456789abcdef";
    out += '"';
    size_t run = 0;  // start of the run of characters that need no escape
    for (size_t i = 0; i < s.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c >= 0x20 && c != '"' && c != '\\') continue;
        out.append(s.data() + run, i - run);
        run = i + 1;
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            default:
                out += "\\u00";
                out += hex[c >> 4];
                out += hex[c & 0xf];
        }
    }
    out.append(s.data() + run, s.size() - run);
    out += '"';
}

// The 1-based line and column of byte `offset` of `text`.
std::pair<size_t, size_t> line_col(std::string_view text, size_t offset) {
    offset = std::min(offset, text.size());
    size_t line = 1, col = 1;
    for (size_t i = 0; i < offset; ++i) {
        if (text[i] == '\n') { ++line; col = 1; } else { ++col; }
    }
    return {line, col};
}

[[noreturn]] void fail_at(const std::string& name, std::string_view text, size_t offset,
                          const std::string& message) {
    auto lc = line_col(text, offset);
    throw ConversionError(format_error_with_location("YAML parsing error", name, lc.first, lc.second)
                          + ": " + message);
}

// YAML 1.2's c-printable: tab, line feed, carriage return, and every other character but
// the C0 and C1 controls (NEL excepted), DEL, surrogates, U+FFFE and U+FFFF.
bool is_printable(char32_t c) {
    if (c < 0x20) return c == 0x09 || c == 0x0a || c == 0x0d;
    if (c >= 0x7f && c <= 0x9f) return c == 0x85;
    return c != 0xfffe && c != 0xffff;
}

// Returns the offset of the first byte that is not well-formed UTF-8 or, when `printable`,
// does not start a c-printable character; npos when there is none.
size_t invalid_text(std::string_view s, bool printable) {
    const auto* p = reinterpret_cast<const unsigned char*>(s.data());
    const size_t n = s.size();
    size_t i = 0;
    while (i < n) {
        const unsigned char c = p[i];
        size_t len = 1;
        char32_t cp = c;
        if (c >= 0x80) {
            unsigned char lo = 0x80, hi = 0xbf;  // range of the second byte
            if (c >= 0xc2 && c <= 0xdf) { len = 2; cp = c & 0x1f; }
            else if (c == 0xe0) { len = 3; cp = c & 0x0f; lo = 0xa0; }
            else if (c == 0xed) { len = 3; cp = c & 0x0f; hi = 0x9f; }  // no surrogates
            else if (c >= 0xe1 && c <= 0xef) { len = 3; cp = c & 0x0f; }
            else if (c == 0xf0) { len = 4; cp = c & 0x07; lo = 0x90; }
            else if (c >= 0xf1 && c <= 0xf3) { len = 4; cp = c & 0x07; }
            else if (c == 0xf4) { len = 4; cp = c & 0x07; hi = 0x8f; }  // up to U+10FFFF
            else return i;
            if (i + len > n || p[i + 1] < lo || p[i + 1] > hi) return i;
            for (size_t k = 1; k < len; ++k) {
                if (p[i + k] < 0x80 || p[i + k] > 0xbf) return i;
                cp = (cp << 6) | (p[i + k] & 0x3f);
            }
        }
        if (printable && !is_printable(cp)) return i;
        i += len;
    }
    return std::string_view::npos;
}

class DocumentWriter {
public:
    DocumentWriter(const ryml::Tree& tree, const std::string& name, std::string_view source,
                   const char* buffer)
        : t_(tree), name_(name), source_(source), buffer_(buffer) {}

    // Appends the JSON of document `id`; returns whether a mapping in it repeats a key.
    bool write(std::string& out, id_type id) {
        anchors_.clear();  // anchors do not cross documents
        duplicate_keys_ = false;
        value(out, id);
        return duplicate_keys_;
    }

private:
    const ryml::Tree& t_;
    const std::string& name_;
    std::string_view source_;  // the text as read, for error locations
    const char* buffer_;       // the copy parsed in place; scalars point into it
    std::unordered_map<std::string_view, std::string> anchors_;
    bool duplicate_keys_ = false;

    [[noreturn]] void fail(csubstr at, const std::string& message) const {
        size_t offset = 0;
        if (at.str >= buffer_ && at.str <= buffer_ + source_.size()) {
            offset = static_cast<size_t>(at.str - buffer_);
        }
        fail_at(name_, source_, offset, message);
    }

    static bool is_styled(bool literal, bool folded, bool quoted) {
        return literal || folded || quoted;
    }

    // The text of a scalar. The input is valid UTF-8, so only an escape of a double-quoted
    // scalar can make it invalid (`\ud800`, a surrogate): package:yaml refuses those too.
    std::string_view text(csubstr s, bool double_quoted) const {
        if (double_quoted && invalid_text(view(s), false) != std::string_view::npos) {
            fail(s, "invalid Unicode character escape");
        }
        return view(s);
    }

    // YAML never lets a plain scalar start with "- ", "? " or ": ", or hold ": " or " #"
    // (space or tab). rapidyaml 0.16 still returns such a scalar when a tab follows an
    // indicator (`key:<TAB>value` on a nested line, `-<TAB>item`), where package:yaml reads a
    // mapping or a sequence: fail instead of returning different data.
    void check_plain(csubstr s) const {
        const std::string_view t = view(s);
        auto blank = [&](size_t i) { return i < t.size() && (t[i] == ' ' || t[i] == '\t'); };
        bool bad = t.size() >= 2 && (t[0] == '-' || t[0] == '?' || t[0] == ':') && blank(1);
        for (size_t i = 0; !bad && i + 1 < t.size(); ++i) {
            bad = (t[i] == ':' && blank(i + 1)) || (blank(i) && t[i + 1] == '#');
        }
        if (bad) {
            fail(s, "a plain scalar cannot read as \"" + std::string(t) +
                    "\" (is a tab used after ':' or '-'?)");
        }
    }

    // A scalar is resolved by the core schema when it is plain and untagged, or tagged
    // !!null or !!bool; anything else (quoted, block, !!str, other tags) is a string.
    static bool resolves(bool styled, csubstr tag) {
        if (tag.empty()) return !styled;
        const ryml::YamlTag_e t = ryml::to_tag(tag);
        return t == ryml::TAG_NULL || t == ryml::TAG_BOOL;
    }

    void scalar(std::string& out, std::string_view text, bool is_nil, bool resolve) {
        if (resolve) {
            if (is_nil || is_null_text(text)) { out += "null"; return; }
            const int b = bool_value(text);
            if (b >= 0) { out += b ? "true" : "false"; return; }
        }
        append_json_string(out, text);
    }

    // The JSON object key for the key of node `id`.
    std::string_view key_name(id_type id) {
        const NodeType ty = t_.type(id);
        if (ty.is_key_ref()) {
            fail(t_.key_ref(id), "an alias cannot be used as a mapping key");
        }
        const std::string_view key = text(t_.key(id), ty.is_key_dquo());
        const bool styled = is_styled(ty.is_key_literal(), ty.is_key_folded(), ty.is_key_quoted());
        if (!styled) check_plain(t_.key(id));
        const csubstr tag = ty.has_key_tag() ? t_.key_tag(id) : csubstr();
        if (resolves(styled, tag) && (ty.key_is_null() || is_null_text(key))) {
            return "null";
        }
        if (ty.has_key_anchor()) {
            std::string json;
            scalar(json, key, ty.key_is_null(), resolves(styled, tag));
            anchors_[view(t_.key_anchor(id))] = std::move(json);
        }
        return key;
    }

    void value(std::string& out, id_type id) {
        const NodeType ty = t_.type(id);
        if (ty.is_val_ref()) {
            const csubstr ref = t_.val_ref(id);
            auto it = anchors_.find(view(ref));
            if (it == anchors_.end()) fail(ref, "undefined alias '*" + std::string(view(ref)) + "'");
            out += it->second;
            return;
        }
        const size_t start = out.size();
        if (ty.is_map()) {
            std::vector<std::string_view> keys;
            out += '{';
            for (id_type ch = t_.first_child(id); ch != ryml::NONE; ch = t_.next_sibling(ch)) {
                if (ch != t_.first_child(id)) out += ',';
                const std::string_view key = key_name(ch);
                keys.push_back(key);
                append_json_string(out, key);
                out += ':';
                value(out, ch);
            }
            out += '}';
            std::sort(keys.begin(), keys.end());
            if (std::adjacent_find(keys.begin(), keys.end()) != keys.end()) duplicate_keys_ = true;
        } else if (ty.is_seq()) {
            out += '[';
            for (id_type ch = t_.first_child(id); ch != ryml::NONE; ch = t_.next_sibling(ch)) {
                if (ch != t_.first_child(id)) out += ',';
                value(out, ch);
            }
            out += ']';
        } else if (ty.has_val()) {
            const bool styled = is_styled(ty.is_val_literal(), ty.is_val_folded(), ty.is_val_quoted());
            if (!styled) check_plain(t_.val(id));
            const csubstr tag = ty.has_val_tag() ? t_.val_tag(id) : csubstr();
            scalar(out, text(t_.val(id), ty.is_val_dquo()), ty.val_is_null(), resolves(styled, tag));
        } else {
            out += "null";
        }
        if (ty.has_val_anchor()) {
            anchors_[view(t_.val_anchor(id))] = out.substr(start);
        }
    }
};

void append_line(std::string& out, const std::string& name, size_t index, bool duplicate_keys,
                 std::string_view data) {
    out += "{\"file\":";
    append_json_string(out, name);
    out += ",\"doc\":";
    out += std::to_string(index);
    out += duplicate_keys ? ",\"duplicate_keys\":true,\"data\":" : ",\"duplicate_keys\":false,\"data\":";
    out += data;
    out += "}\n";
}

} // namespace

void BatchConverter::append_documents(std::string& out, const std::string& name,
                                      const char* yaml, size_t size) {
    setup_error_handlers();
    const std::string_view source(yaml, size);
    const size_t bad = invalid_text(source, true);
    if (bad != std::string_view::npos) {
        fail_at(name, source, bad, "invalid UTF-8 or a character YAML does not allow");
    }

    // Parse a copy in place, so that error locations can be read from the text as given.
    std::string buffer(source);
    ryml::Tree tree;
    tree.reserve(std::max<size_t>(64, size / 32));
    tree.reserve_arena(size / 8);
    try {
        ryml::parse_in_place(ryml::csubstr(name.data(), name.size()),
                             ryml::substr(buffer.data(), buffer.size()), &tree);
    } catch (const ConversionError&) {
        throw;
    } catch (const std::exception& e) {
        throw ConversionError("YAML parsing error in file '" + name + "': " + e.what());
    }

    std::vector<id_type> docs;
    const id_type root = tree.root_id();
    if (tree.is_stream(root)) {
        for (id_type d = tree.first_child(root); d != ryml::NONE; d = tree.next_sibling(d)) {
            docs.push_back(d);
        }
    } else {
        docs.push_back(root);
    }

    DocumentWriter writer(tree, name, source, buffer.data());
    std::string lines, data;
    if (docs.empty()) append_line(lines, name, 0, false, "null");
    for (size_t i = 0; i < docs.size(); ++i) {
        data.clear();
        const bool dup = writer.write(data, docs[i]);
        append_line(lines, name, i, dup, data);
    }
    out += lines;
}

std::string BatchConverter::convert(const std::string& yaml, const std::string& name) {
    std::string out;
    append_documents(out, name, yaml.data(), yaml.size());
    return out;
}

} // namespace yaml2json
