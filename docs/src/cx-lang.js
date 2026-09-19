// CodeMirror 6 language support for C*.
// The tokenizer is based on the CodeMirror Go mode by Marijn Haverbeke
// and others:
//
// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE

import { HighlightStyle, LanguageSupport, StreamLanguage } from "@codemirror/language";
import { EditorView } from "@codemirror/view";
import { tags } from "@lezer/highlight";

const keywords = {
    "break": true, "case": true, "cast": true, "catch": true, "class": true, "const": true, "continue": true, "default": true,
    "defer": true, "deinit": true, "do": true, "else": true, "enum": true, "extern": true, "fallthrough": true, "for": true,
    "if": true, "import": true, "in": true, "init": true, "interface": true, "operator": true, "private": true, "public": true, "return": true,
    "sizeof": true, "static": true, "struct": true, "switch": true, "this": true, "throw": true, "throws": true, "try": true, "typealias": true,
    "undefined": true, "var": true, "while": true, "bool": true, "float": true, "float32": true, "float64": true,
    "float80": true, "int8": true, "int16": true, "int32": true, "int64": true, "uint8": true, "uint16": true, "uint32": true,
    "uint64": true, "int": true, "uint": true, "uintptr": true, "char": true, "void": true
};

const atoms = {
    "true": true, "false": true, "null": true
};

const isOperatorChar = /[+\-*&^%:=<>!|\/]/;

function tokenBase(stream, state) {
    const ch = stream.next();
    if (ch == '"' || ch == "'" || ch == "`") {
        state.tokenize = tokenString(ch);
        return state.tokenize(stream, state);
    }
    if (/[\d]/.test(ch)) {
        if (ch == "0") {
            stream.match(/^[0-9]*\.?[0-9]+([eE][\-+]?[0-9]+)?/) || stream.match(/^[xX][0-9a-fA-F]+/) || stream.match(/^0[0-7]+/);
        } else {
            stream.match(/^[0-9]*\.?[0-9]+([eE][\-+]?[0-9]+)?/);
        }
        return "number";
    }
    if (/[\[\]{}\(\),;\:\.]/.test(ch)) {
        state.curPunc = ch;
        return null;
    }
    if (ch == "/") {
        if (stream.eat("*")) {
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
    stream.eatWhile(/[\w\$_\xa1-\uffff]/);
    const cur = stream.current();
    if (keywords.propertyIsEnumerable(cur)) {
        if (cur == "case" || cur == "default") state.curPunc = "case";
        return "keyword";
    }
    if (atoms.propertyIsEnumerable(cur)) return "atom";
    return "variable";
}

function tokenString(quote) {
    return function (stream, state) {
        let escaped = false, next, end = false;
        while ((next = stream.next()) != null) {
            if (next == quote && !escaped) {
                end = true;
                break;
            }
            escaped = !escaped && quote != "`" && next == "\\";
        }
        if (end || !(escaped || quote == "`"))
            state.tokenize = tokenBase;
        return "string";
    };
}

function tokenComment(stream, state) {
    let maybeEnd = false, ch;
    while (ch = stream.next()) {
        if (ch == "/" && maybeEnd) {
            state.tokenize = tokenBase;
            break;
        }
        maybeEnd = (ch == "*");
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
    const t = state.context.type;
    if (t == ")" || t == "]" || t == "}")
        state.indented = state.context.indented;
    return state.context = state.context.prev;
}

const cxStreamParser = {
    name: "cx",

    startState(indentUnit) {
        return {
            tokenize: null,
            context: new Context(-indentUnit, 0, "top", false),
            indented: 0,
            startOfLine: true
        };
    },

    token(stream, state) {
        let ctx = state.context;
        if (stream.sol()) {
            if (ctx.align == null) ctx.align = false;
            state.indented = stream.indentation();
            state.startOfLine = true;
            if (ctx.type == "case") ctx.type = "}";
        }
        if (stream.eatSpace()) return null;
        state.curPunc = null;
        const style = (state.tokenize || tokenBase)(stream, state);
        if (style == "comment") return style;
        if (ctx.align == null) ctx.align = true;

        if (state.curPunc == "{") pushContext(state, stream.column(), "}");
        else if (state.curPunc == "[") pushContext(state, stream.column(), "]");
        else if (state.curPunc == "(") pushContext(state, stream.column(), ")");
        else if (state.curPunc == "case") ctx.type = "case";
        else if (state.curPunc == "}" && ctx.type == "}") ctx = popContext(state);
        else if (state.curPunc == ctx.type) popContext(state);
        state.startOfLine = false;
        return style;
    },

    indent(state, textAfter, context) {
        if (state.tokenize != tokenBase && state.tokenize != null) return 0;
        const ctx = state.context, firstChar = textAfter && textAfter.charAt(0);
        if (ctx.type == "case" && /^(?:case|default)\b/.test(textAfter)) {
            state.context.type = "}";
            return ctx.indented;
        }
        const closing = firstChar == ctx.type;
        if (ctx.align) return ctx.column + (closing ? 0 : 1);
        else return ctx.indented + (closing ? 0 : context.unit);
    },

    languageData: {
        commentTokens: { line: "//", block: { open: "/*", close: "*/" } }
    }
};

export const cxLanguage = StreamLanguage.define(cxStreamParser);

export function cx() {
    return new LanguageSupport(cxLanguage);
}

export const cxHighlightStyle = HighlightStyle.define([
    { tag: tags.keyword, color: "var(--blue)" },
    { tag: tags.atom, color: "var(--blue)" },
    { tag: tags.number, color: "var(--number-color)" },
    { tag: tags.string, color: "var(--string-color)" },
    { tag: tags.comment, color: "var(--text-color-subtle)" },
    { tag: tags.variableName, color: "var(--text-color)" },
    { tag: tags.operator, color: "var(--text-color)" },
]);

// Matches the site palette in both color schemes via CSS variables.
// Selectors mirror CodeMirror's base theme so these rules win ties.
export const cxTheme = EditorView.theme({
    "&": {
        backgroundColor: "transparent",
        color: "var(--text-color)",
        fontSize: "13px",
        height: "auto",
        width: "100%",
    },
    "&.cm-focused": {
        outline: "none",
    },
    ".cm-scroller": {
        fontFamily: "inherit",
        lineHeight: "1.6",
    },
    ".cm-content": {
        padding: "12px 0",
        caretColor: "var(--text-color)",
    },
    ".cm-line": {
        padding: "0 18px",
    },
    ".cm-selectionBackground": {
        backgroundColor: "var(--background-color-3)",
    },
    "&.cm-focused > .cm-scroller > .cm-selectionLayer .cm-selectionBackground": {
        backgroundColor: "var(--background-color-3)",
    },
    ".cm-cursor": {
        borderLeftColor: "var(--text-color)",
    },
});
