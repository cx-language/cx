// Regression tests for the cx CodeMirror mode (website/cx.js). Runs the
// tokenizer against a fake StringStream, since the repo has no browser harness.
// Run: node website/test-cx-mode.mjs
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import vm from "node:vm";

const SITE = path.dirname(fileURLToPath(import.meta.url));
const SRC = fs.readFileSync(path.join(SITE, "cx.js"), "utf8");

let failures = 0;
function check(name, cond, detail = "") {
    if (!cond) {
        failures++;
        console.log(`FAIL ${name} ${detail}`);
    }
}

let factory = null;
const sandbox = { CodeMirror: { defineMode(_name, fn) { factory = fn; } } };
vm.createContext(sandbox);
vm.runInContext(SRC, sandbox, { filename: "cx.js" });
check("mode registered", typeof factory == "function");

const mode = factory({ indentUnit: 4 });

// Minimal StringStream matching the CodeMirror 5 API surface the cx mode uses.
class Stream {
    constructor(line) {
        this.string = line;
        this.pos = 0;
        this.start = 0;
    }
    sol() { return this.pos == 0; }
    eol() { return this.pos >= this.string.length; }
    next() {
        if (this.pos < this.string.length) return this.string.charAt(this.pos++);
    }
    eat(match) {
        const ch = this.string.charAt(this.pos);
        const ok = typeof match == "string" ? ch == match : ch && match.test(ch);
        if (ok) {
            this.pos++;
            return ch;
        }
    }
    eatWhile(match) {
        const start = this.pos;
        while (this.eat(match));
        return this.pos > start;
    }
    eatSpace() { return this.eatWhile(/[\s ]/); }
    skipToEnd() { this.pos = this.string.length; }
    match(pattern, consume) {
        const m = this.string.slice(this.pos).match(pattern);
        if (m && m.index == 0) {
            if (consume !== false) this.pos += m[0].length;
            return m;
        }
        return null;
    }
    current() { return this.string.slice(this.start, this.pos); }
    column() { return countColumn(this.string.slice(0, this.start)); }
    indentation() { return countColumn(/^\s*/.exec(this.string)[0]); }
}

// CodeMirror counts tab-expanded columns with a default tab size of 4.
function countColumn(text) {
    let col = 0;
    for (const ch of text) col = ch == "\t" ? col + 4 - (col % 4) : col + 1;
    return col;
}

function tokenize(text) {
    const state = mode.startState(0);
    const out = [];
    for (const line of text.split("\n")) {
        const stream = new Stream(line);
        while (!stream.eol()) {
            stream.start = stream.pos;
            const style = mode.token(stream, state);
            if (stream.pos == stream.start) throw new Error("mode stalled on: " + JSON.stringify(line));
            const tok = stream.current();
            if (!/^\s*$/.test(tok)) out.push([tok, style || null]);
        }
    }
    return out;
}

function checkTokens(name, source, want) {
    const got = tokenize(source);
    const ok = got.length == want.length && got.every((t, i) => t[0] == want[i][0] && t[1] == want[i][1]);
    check(name, ok, ok ? "" : `want ${JSON.stringify(want)} got ${JSON.stringify(got)}`);
}

checkTokens("definition", "void main() {", [
    ["void", "type"], ["main", "def"], ["(", null], [")", null], ["{", null],
]);
checkTokens("keyword wins over call lookahead", "while (x) {", [
    ["while", "keyword"], ["(", null], ["x", "variable"], [")", null], ["{", null],
]);
checkTokens("type wins over call lookahead", "int(x)", [
    ["int", "type"], ["(", null], ["x", "variable"], [")", null],
]);
checkTokens("constructor calls are plain", "var v = Vec2(1, 2);", [
    ["var", "keyword"], ["v", "variable"], ["=", "operator"], ["Vec2", "variable"],
    ["(", null], ["1", "number"], [",", null], ["2", "number"], [")", null], [";", null],
]);
checkTokens("method calls are plain", "greeting.size ()", [
    ["greeting", "variable"], [".", null], ["size", "variable"], ["(", null], [")", null],
]);
checkTokens("member call on literal", "10.toString()", [
    ["10", "number"], [".", null], ["toString", "variable"], ["(", null], [")", null],
]);
checkTokens("bare call is plain", "main();", [
    ["main", "variable"], ["(", null], [")", null], [";", null],
]);
checkTokens("nested calls are plain", "foo(bar());", [
    ["foo", "variable"], ["(", null], ["bar", "variable"], ["(", null], [")", null], [")", null], [";", null],
]);
checkTokens("call after keyword", "if (x) { y(); }", [
    ["if", "keyword"], ["(", null], ["x", "variable"], [")", null], ["{", null],
    ["y", "variable"], ["(", null], [")", null], [";", null], ["}", null],
]);
checkTokens("definition with params", "int add(int a, int b) {", [
    ["int", "type"], ["add", "def"], ["(", null], ["int", "type"], ["a", "variable"],
    [",", null], ["int", "type"], ["b", "variable"], [")", null], ["{", null],
]);
checkTokens("generic return misreads as call", "List<int> get() {", [
    ["List", "type"], ["<", "operator"], ["int", "type"], [">", "operator"],
    ["get", "variable"], ["(", null], [")", null], ["{", null],
]);
checkTokens("atoms", "return true; // also false null undefined", [
    ["return", "keyword"], ["true", "atom"], [";", null], ["// also false null undefined", "comment"],
]);
checkTokens("null check", "if x == null {", [
    ["if", "keyword"], ["x", "variable"], ["==", "operator"], ["null", "atom"], ["{", null],
]);
checkTokens("nullable type", "string? name = \"hi\";", [
    ["string", "type"], ["?", "operator"], ["name", "variable"], ["=", "operator"], ["\"hi\"", "string"], [";", null],
]);
checkTokens("generics", "List<int> l;", [
    ["List", "type"], ["<", "operator"], ["int", "type"], [">", "operator"], ["l", "variable"], [";", null],
]);
checkTokens("numbers", "0xFF 0o17 0b101 1_000 3.14 1e5 2.5e-3", [
    ["0xFF", "number"], ["0o17", "number"], ["0b101", "number"],
    ["1_000", "number"], ["3.14", "number"], ["1e5", "number"], ["2.5e-3", "number"],
]);
checkTokens("bad exponent splits", "1e+", [["1", "number"], ["e", "variable"], ["+", "operator"]]);
checkTokens("member access on literal", "0.foo", [["0", "number"], [".", null], ["foo", "variable"]]);
checkTokens("range on literal", "r = 0..10;", [
    ["r", "variable"], ["=", "operator"], ["0", "number"], [".", null], [".", null], ["10", "number"], [";", null],
]);
checkTokens("operators", "a = ~b ? c : d;", [
    ["a", "variable"], ["=", "operator"], ["~", "operator"], ["b", "variable"],
    ["?", "operator"], ["c", "variable"], [":", null], ["d", "variable"], [";", null],
]);
checkTokens("null-conditional", "q = shape?.field ?? fallback;", [
    ["q", "variable"], ["=", "operator"], ["shape", "variable"], ["?", "operator"], [".", null],
    ["field", "variable"], ["??", "operator"], ["fallback", "variable"], [";", null],
]);
checkTokens("attribute", "@test", [["@test", "attribute"]]);
checkTokens("directive", "#if FOO", [["#if", "keyword"], ["FOO", "type"]]);
checkTokens("bare dot before exponent", "1.e5", [["1.e5", "number"]]);
checkTokens("uppercase prefix splits", "0XFF", [["0", "number"], ["XFF", "type"]]);
checkTokens("float separators split", "1.0_5", [["1.0", "number"], ["_5", "variable"]]);
checkTokens("lone at", "@", [["@", "attribute"]]);
checkTokens("unterminated block comment", "/* foo\nbar", [
    ["/* foo", "comment"], ["bar", "comment"],
]);
checkTokens("char literal", "char c = 'x';", [
    ["char", "type"], ["c", "variable"], ["=", "operator"], ["'x'", "string"], [";", null],
]);
checkTokens("escapes stay in string", "\"a \\\" b\"", [["\"a \\\" b\"", "string"]]);
checkTokens("interpolation is plain string", "\"a {b} c\"", [["\"a {b} c\"", "string"]]);
checkTokens("new keywords", "then union implicit", [
    ["then", "keyword"], ["union", "keyword"], ["implicit", "keyword"],
]);
checkTokens("stale words are identifiers", "class try catch uintptr", [
    ["class", "variable"], ["try", "variable"], ["catch", "variable"], ["uintptr", "variable"],
]);
checkTokens("operator overload", "Vec2 operator+(Vec2 a, Vec2 b) {", [
    ["Vec2", "type"], ["operator", "keyword"], ["+", "operator"], ["(", null],
    ["Vec2", "type"], ["a", "variable"], [",", null], ["Vec2", "type"], ["b", "variable"],
    [")", null], ["{", null],
]);
checkTokens("bare operator is identifier", "var operator = 1;", [
    ["var", "keyword"], ["operator", "variable"], ["=", "operator"], ["1", "number"], [";", null],
]);
checkTokens("equality overload", "bool operator==(Vec2 a, Vec2 b) {", [
    ["bool", "type"], ["operator", "keyword"], ["==", "operator"], ["(", null],
    ["Vec2", "type"], ["a", "variable"], [",", null], ["Vec2", "type"], ["b", "variable"],
    [")", null], ["{", null],
]);
checkTokens("subscript overload", "int operator[](Vec2 v, int i) {", [
    ["int", "type"], ["operator", "keyword"], ["[", null], ["]", null], ["(", null],
    ["Vec2", "type"], ["v", "variable"], [",", null], ["int", "type"], ["i", "variable"],
    [")", null], ["{", null],
]);
checkTokens("nested block comment", "/* a /* b */ c */ x", [
    ["/* a /* b */ c */", "comment"], ["x", "variable"],
]);
checkTokens("multiline nested comment", "/* outer\nstill /* nested\n*/ still comment\n*/ var y = 1;", [
    ["/* outer", "comment"], ["still /* nested", "comment"], ["*/ still comment", "comment"],
    ["*/", "comment"], ["var", "keyword"], ["y", "variable"], ["=", "operator"], ["1", "number"], [";", null],
]);
checkTokens("unterminated string resets", "var s = \"abc\ndef", [
    ["var", "keyword"], ["s", "variable"], ["=", "operator"],
    ["\"abc", "string"], ["def", "variable"],
]);
check("no backtick auto-close", mode.closeBrackets === "()[]{}''\"\"");

if (failures) {
    console.log(`${failures} FAILURES`);
    process.exit(1);
}
console.log("ALL PASS");
