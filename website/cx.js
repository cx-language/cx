// CodeMirror mode for cx.
// Based on the CodeMirror Go mode by Marijn Haverbeke and others:
//
// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: http://codemirror.net/LICENSE

(function (mod) {
    if (typeof exports == "object" && typeof module == "object") // CommonJS
        mod(require("../../lib/codemirror"));
    else if (typeof define == "function" && define.amd) // AMD
        define(["../../lib/codemirror"], mod);
    else // Plain browser env
        mod(CodeMirror);
})(function (CodeMirror) {
    "use strict";

    CodeMirror.defineMode("cx", function (config) {
        var indentUnit = config.indentUnit;

        // Mirrors the keyword table in src/parser/lex.cpp.
        var keywords = {
            "break": true, "case": true, "const": true, "continue": true, "default": true,
            "defer": true, "do": true, "else": true, "enum": true, "extern": true,
            "for": true, "if": true, "implicit": true, "import": true, "in": true,
            "interface": true, "is": true, "private": true, "public": true, "return": true,
            "sizeof": true, "struct": true, "switch": true, "then": true, "this": true,
            "union": true, "using": true, "var": true, "while": true,
            "#if": true, "#else": true, "#endif": true
        };

        var atoms = {
            "true": true, "false": true, "null": true, "undefined": true
        };

        // Reserved type names. string and never are contextual, but shadowing
        // them is pathological, so they highlight as types unconditionally.
        var builtinTypes = {
            "void": true, "bool": true, "char": true, "string": true, "never": true,
            "int": true, "int8": true, "int16": true, "int32": true, "int64": true,
            "uint": true, "uint8": true, "uint16": true, "uint32": true, "uint64": true,
            "sbyte": true, "byte": true, "float": true, "float32": true, "float64": true,
            "float80": true, "double": true,
            "c_size_t": true, "c_schar": true, "c_uchar": true, "c_short": true, "c_ushort": true,
            "c_int": true, "c_uint": true, "c_long": true, "c_ulong": true,
            "c_longlong": true, "c_ulonglong": true, "c_float": true, "c_double": true
        };

        var isOperatorChar = /[+\-*&^%:=<>!|\/?~]/;
        var isWordChar = /[\w\$_\xa1-\uffff]/;

        var curPunc;

        function tokenBase(stream, state) {
            var ch = stream.next();
            if (ch == '"' || ch == "'") {
                state.tokenize = tokenString(ch);
                return state.tokenize(stream, state);
            }
            if (ch == "@") {
                stream.eatWhile(isWordChar);
                return "attribute";
            }
            if (/[\d]/.test(ch)) {
                if (ch == "0" && (stream.match(/^x[0-9a-fA-F_]+/) || stream.match(/^o[0-7_]+/) || stream.match(/^b[01_]+/))) {
                    return "number";
                }
                stream.match(/^[\d_]*/);
                // The fraction requires a digit after the dot, so member
                // access (0.foo) and ranges (0..10) lex separately. A bare
                // dot survives only before an exponent (1.e5).
                if (!stream.match(/^\.\d+/)) stream.match(/^\.(?=[eE])/);
                // Separators are integers-only; floats reject them.
                stream.match(/^[eE][+-]?\d+/);
                return "number";
            }
            if (/[\[\]{}\(\),;\:\.]/.test(ch)) {
                curPunc = ch;
                return null;
            }
            if (ch == "/") {
                if (stream.eat("*")) {
                    state.commentDepth = 1;
                    state.tokenize = tokenComment;
                    return tokenComment(stream, state);
                }
                if (stream.eat("/")) {
                    stream.skipToEnd();
                    return "comment";
                }
            }
            if (isOperatorChar.test(ch)) {
                stream.eatWhile(isOperatorChar);
                return "operator";
            }
            stream.eatWhile(isWordChar);
            var cur = stream.current();
            if (keywords.propertyIsEnumerable(cur)) {
                if (cur == "case" || cur == "default") curPunc = "case";
                return "keyword";
            }
            if (atoms.propertyIsEnumerable(cur)) return "atom";
            if (builtinTypes.propertyIsEnumerable(cur)) return "type";
            // operator is contextual: a keyword only in overload declarations.
            if (cur == "operator" && stream.match(/^\s*(\[|==|!=|<=|>=|<|>|[+\-*/%])/, false)) return "keyword";
            // Calls and definitions alike: a word followed by `(`.
            if (stream.match(/^\s*\(/, false)) return "def";
            // User type names start with an uppercase letter by convention.
            if (/^[A-Z]/.test(cur)) return "type";
            return "variable";
        }

        function tokenString(quote) {
            return function (stream, state) {
                var escaped = false, next, end = false;
                while ((next = stream.next()) != null) {
                    if (next == quote && !escaped) {
                        end = true;
                        break;
                    }
                    escaped = !escaped && next == "\\";
                }
                if (end || !escaped)
                    state.tokenize = tokenBase;
                return "string";
            };
        }

        function tokenComment(stream, state) {
            var ch;
            while ((ch = stream.next()) != null) {
                if (ch == "*" && stream.eat("/")) {
                    if (--state.commentDepth <= 0) {
                        state.tokenize = tokenBase;
                        break;
                    }
                } else if (ch == "/" && stream.eat("*")) {
                    state.commentDepth++;
                }
            }
            return "comment";
        }

        function Context(indented, column, type, align, prev) {
            this.indented = indented;
            this.column = column;
            this.type = type;
            this.align = align;
            this.prev = prev;
        }

        function pushContext(state, col, type) {
            return state.context = new Context(state.indented, col, type, null, state.context);
        }

        function popContext(state) {
            if (!state.context.prev) return;
            var t = state.context.type;
            if (t == ")" || t == "]" || t == "}")
                state.indented = state.context.indented;
            return state.context = state.context.prev;
        }

        // Interface

        return {
            startState: function (basecolumn) {
                return {
                    tokenize: null,
                    context: new Context((basecolumn || 0) - indentUnit, 0, "top", false),
                    indented: 0,
                    startOfLine: true,
                    commentDepth: 0
                };
            },

            token: function (stream, state) {
                var ctx = state.context;
                if (stream.sol()) {
                    if (ctx.align == null) ctx.align = false;
                    state.indented = stream.indentation();
                    state.startOfLine = true;
                    if (ctx.type == "case") ctx.type = "}";
                }
                if (stream.eatSpace()) return null;
                curPunc = null;
                var style = (state.tokenize || tokenBase)(stream, state);
                if (style == "comment") return style;
                if (ctx.align == null) ctx.align = true;

                if (curPunc == "{") pushContext(state, stream.column(), "}");
                else if (curPunc == "[") pushContext(state, stream.column(), "]");
                else if (curPunc == "(") pushContext(state, stream.column(), ")");
                else if (curPunc == "case") ctx.type = "case";
                else if (curPunc == "}" && ctx.type == "}") ctx = popContext(state);
                else if (curPunc == ctx.type) popContext(state);
                state.startOfLine = false;
                return style;
            },

            indent: function (state, textAfter) {
                if (state.tokenize != tokenBase && state.tokenize != null) return 0;
                var ctx = state.context, firstChar = textAfter && textAfter.charAt(0);
                if (ctx.type == "case" && /^(?:case|default)\b/.test(textAfter)) {
                    state.context.type = "}";
                    return ctx.indented;
                }
                var closing = firstChar == ctx.type;
                if (ctx.align) return ctx.column + (closing ? 0 : 1);
                else return ctx.indented + (closing ? 0 : indentUnit);
            },

            electricChars: "{}):",
            closeBrackets: "()[]{}''\"\"",
            fold: "brace",
            blockCommentStart: "/*",
            blockCommentEnd: "*/",
            lineComment: "//"
        };
    });

});
