// Parses compiler diagnostics from playground output.
//
// The compiler reports errors as `main.cx:<line>:<column>: <message>`,
// optionally followed by the offending source line and a caret line
// pointing at the error column.

export function parseDiagnostics(output) {
    const diagnostics = [];
    const regex = /^main\.cx:(\d+):(\d+): (.*)(?:\n.*\n([ \t]*)\^)?/gm;
    let match;
    while ((match = regex.exec(output))) {
        const [, line, column, message, indent] = match;
        diagnostics.push({
            line: Number(line),
            column: Number(column),
            message,
            indent: indent || "",
            kind: message.startsWith("warning") ? "warning" : "error",
        });
    }
    return diagnostics;
}

export function formatDiagnostic(diagnostic) {
    return diagnostic.indent + "^ " + diagnostic.message;
}
