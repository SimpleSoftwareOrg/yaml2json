#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vector>
#include "BatchConverter.h"
#include "ErrorHandler.h"

using namespace yaml2json;

namespace {

// The "data" of the only document of `yaml`.
std::string data_of(const std::string& yaml) {
    const std::string line = BatchConverter::convert(yaml, "t.yaml");
    const std::string head = "{\"file\":\"t.yaml\",\"doc\":0,\"duplicate_keys\":false,\"data\":";
    EXPECT_EQ(line.compare(0, head.size(), head), 0) << line;
    EXPECT_EQ(line.find('\n'), line.size() - 1) << "more than one document: " << line;
    return line.substr(head.size(), line.size() - head.size() - 2);
}

bool has_duplicate_keys(const std::string& yaml) {
    const std::string line = BatchConverter::convert(yaml, "t.yaml");
    return line.find("\"duplicate_keys\":true") != std::string::npos;
}

std::string error_of(const std::string& yaml) {
    try {
        BatchConverter::convert(yaml, "bad.yaml");
    } catch (const ConversionError& e) {
        return e.what();
    }
    return "no error";
}

} // namespace

// What each scalar of `v: <scalar>` becomes: booleans and nulls of YAML 1.2's core schema
// are typed, every other scalar is a string holding its text.
TEST(BatchConverterTest, ScalarTable) {
    const std::vector<std::pair<std::string, std::string>> table = {
        {"true", "true"}, {"True", "true"}, {"TRUE", "true"},
        {"false", "false"}, {"False", "false"}, {"FALSE", "false"},
        {"null", "null"}, {"Null", "null"}, {"NULL", "null"}, {"~", "null"}, {"", "null"},
        {"!!bool true", "true"}, {"!!null null", "null"},
        {"\"true\"", "\"true\""}, {"'false'", "\"false\""}, {"\"null\"", "\"null\""},
        {"'~'", "\"~\""}, {"\"\"", "\"\""}, {"!!str true", "\"true\""}, {"!!str", "\"\""},
        {"yes", "\"yes\""}, {"No", "\"No\""}, {"on", "\"on\""}, {"Off", "\"Off\""},
        {"tRUE", "\"tRUE\""}, {"nil", "\"nil\""},
        {"1.10", "\"1.10\""}, {"0x10", "\"0x10\""}, {"0o17", "\"0o17\""}, {"1_000", "\"1_000\""},
        {"123e45", "\"123e45\""}, {"000123", "\"000123\""}, {"+12", "\"+12\""}, {"-0", "\"-0\""},
        {".inf", "\".inf\""}, {"-.Inf", "\"-.Inf\""}, {".nan", "\".nan\""}, {"1e999", "\"1e999\""},
        {"0b101", "\"0b101\""}, {"2024-01-01", "\"2024-01-01\""}, {"12:30:45", "\"12:30:45\""},
        {"^3.0.0", "\"^3.0.0\""}, {"'>=2.12.0 <4.0.0'", "\">=2.12.0 <4.0.0\""}, {"any", "\"any\""},
        {"1234567890123456789012345678901234567890", "\"1234567890123456789012345678901234567890\""},
        {"1234567890123456789012345678901234567890123456789012345678901234",
         "\"1234567890123456789012345678901234567890123456789012345678901234\""},
        {"123456789012345678901234567890123456e789", "\"123456789012345678901234567890123456e789\""},
        {"\"caf\\u00e9\"", "\"caf\xc3\xa9\""}, {"\"a\\tb\\\\c\\\"d\"", "\"a\\tb\\\\c\\\"d\""},
        {"\"\\x01\"", "\"\\u0001\""}, {"'it''s'", "\"it's\""},
        {"plain\n  continued", "\"plain continued\""},
        {"|\n  line 1\n  line 2\n", "\"line 1\\nline 2\\n\""},
        {">-\n  folded\n  text\n", "\"folded text\""},
        {"|\n  true\n", "\"true\\n\""},
    };
    for (const auto& row : table) {
        EXPECT_EQ(data_of("v: " + row.first + "\n"), "{\"v\":" + row.second + "}")
            << "scalar: " << row.first;
    }
}

TEST(BatchConverterTest, KeysAreTheirText) {
    EXPECT_EQ(data_of("1: a\ntrue: b\nnull: c\n1.50: d\n\"q\": e\n"),
              "{\"1\":\"a\",\"true\":\"b\",\"null\":\"c\",\"1.50\":\"d\",\"q\":\"e\"}");
}

TEST(BatchConverterTest, CollectionsKeepTheirOrder) {
    EXPECT_EQ(data_of("b: [1, x, {c: true}]\na:\n  - ~\n  - k: v\nz: {}\ny: []\n"),
              "{\"b\":[\"1\",\"x\",{\"c\":true}],\"a\":[null,{\"k\":\"v\"}],\"z\":{},\"y\":[]}");
}

TEST(BatchConverterTest, CrlfAndByteOrderMark) {
    EXPECT_EQ(data_of("\xef\xbb\xbf" "a: 1\r\nb:\r\n  - x\r\n"), "{\"a\":\"1\",\"b\":[\"x\"]}");
}

TEST(BatchConverterTest, AliasesAreExpandedAndMergeKeysStayKeys) {
    EXPECT_EQ(data_of("a: &v 1.10\nb: *v\nc: &m {x: 0x10}\nd: *m\ne:\n  <<: *m\n  y: 2\n"),
              "{\"a\":\"1.10\",\"b\":\"1.10\",\"c\":{\"x\":\"0x10\"},\"d\":{\"x\":\"0x10\"},"
              "\"e\":{\"<<\":{\"x\":\"0x10\"},\"y\":\"2\"}}");
    EXPECT_EQ(data_of("a: &t true\nb: *t\n"), "{\"a\":true,\"b\":true}");
}

TEST(BatchConverterTest, DuplicateKeys) {
    EXPECT_FALSE(has_duplicate_keys("a: 1\nb: 1\nc: {a: 1}\n"));
    EXPECT_TRUE(has_duplicate_keys("a: 1\nb: 2\na: 3\n"));
    EXPECT_TRUE(has_duplicate_keys("packages:\n  x:\n    version: 1\n    version: 2\n"));
    EXPECT_TRUE(has_duplicate_keys("list:\n  - {a: 1, a: 2}\n"));
    EXPECT_TRUE(has_duplicate_keys("\"a\": 1\na: 2\n"));
    EXPECT_TRUE(has_duplicate_keys("null: 1\n~: 2\n"));
    // Both keys stay in the output; a JSON reader keeps the last.
    EXPECT_EQ(BatchConverter::convert("a: 1\na: 2\n", "d.yaml"),
              "{\"file\":\"d.yaml\",\"doc\":0,\"duplicate_keys\":true,\"data\":{\"a\":\"1\",\"a\":\"2\"}}\n");
}

TEST(BatchConverterTest, EveryDocument) {
    EXPECT_EQ(BatchConverter::convert("a: 1\n---\nb: 2\n", "m.yaml"),
              "{\"file\":\"m.yaml\",\"doc\":0,\"duplicate_keys\":false,\"data\":{\"a\":\"1\"}}\n"
              "{\"file\":\"m.yaml\",\"doc\":1,\"duplicate_keys\":false,\"data\":{\"b\":\"2\"}}\n");
    EXPECT_EQ(BatchConverter::convert("---\n---\na: 1\n", "m.yaml"),
              "{\"file\":\"m.yaml\",\"doc\":0,\"duplicate_keys\":false,\"data\":null}\n"
              "{\"file\":\"m.yaml\",\"doc\":1,\"duplicate_keys\":false,\"data\":{\"a\":\"1\"}}\n");
    EXPECT_EQ(BatchConverter::convert("--- x\n--- [y]\n...\n", "m.yaml"),
              "{\"file\":\"m.yaml\",\"doc\":0,\"duplicate_keys\":false,\"data\":\"x\"}\n"
              "{\"file\":\"m.yaml\",\"doc\":1,\"duplicate_keys\":false,\"data\":[\"y\"]}\n");
}

TEST(BatchConverterTest, EmptyOrCommentOnlyIsOneNullDocument) {
    const std::string null_doc = "{\"file\":\"e.yaml\",\"doc\":0,\"duplicate_keys\":false,\"data\":null}\n";
    EXPECT_EQ(BatchConverter::convert("", "e.yaml"), null_doc);
    EXPECT_EQ(BatchConverter::convert("# nothing here\n\n", "e.yaml"), null_doc);
    EXPECT_EQ(BatchConverter::convert("---\n", "e.yaml"), null_doc);
}

TEST(BatchConverterTest, FileNameIsPrintedAsGiven) {
    EXPECT_EQ(BatchConverter::convert("a: b\n", "dir\\\"q\".yaml"),
              "{\"file\":\"dir\\\\\\\"q\\\".yaml\",\"doc\":0,\"duplicate_keys\":false,"
              "\"data\":{\"a\":\"b\"}}\n");
}

// Invalid YAML fails with a message naming the file and the line.
TEST(BatchConverterTest, MustFail) {
    const std::vector<std::pair<std::string, std::string>> cases = {
        {"a: 1\nb:\n\tc: 2\n", "line 3"},             // tab indentation
        {"PODS:\n\t- a (1.0)\n", "line 2"},          // tab indentation
        {"a: 1\n... text\n", "line 2"},              // text after the document end marker
        {"a: \"x\" y\n", "line 1"},                  // text after a closing quote
        {"a: 'x' y\n", "line 1"},
        {"a: [x, y\n", "line"},                       // unclosed flow sequence
        {"a: {x: 1\n", "line"},                       // unclosed flow mapping
        {"a: b: c\n", "line 1"},                     // a mapping value inside a plain scalar
        {"a: 1\nb: *nope\n", "line 2"},              // undefined alias
        {"*a : 1\n", "line 1"},                      // alias as a key
        {"a: \"\\ud800\"\n", "line 1"},              // escape to a surrogate
        {"a: 1\nb: \xff\n", "line 2"},               // invalid UTF-8
        {"a: 1\nb: \xc0\xaf\n", "line 2"},           // overlong UTF-8
        {"a: x\x7fy\n", "line 1"},                   // DEL is not printable
        {"a: x\x01y\n", "line 1"},                   // C0 control
        {"a: x\xc2\x80y\n", "line 1"},               // C1 control
    };
    for (const auto& c : cases) {
        const std::string err = error_of(c.first);
        EXPECT_NE(err.find("'bad.yaml'"), std::string::npos) << c.first << " -> " << err;
        EXPECT_NE(err.find(c.second), std::string::npos) << c.first << " -> " << err;
    }
}

TEST(BatchConverterTest, FailureLeavesOutputUnchanged) {
    std::string out = "kept\n";
    const std::string bad = "a: [x\n";
    EXPECT_THROW(BatchConverter::append_documents(out, "bad.yaml", bad.data(), bad.size()),
                 ConversionError);
    EXPECT_EQ(out, "kept\n");
}
