// Documentation site editor, bundled into lib/editor.bundle.js.
// Regenerate with `npm run build` (from docs/) after changing docs/src/.

import { defaultKeymap, history, historyKeymap, indentWithTab } from "@codemirror/commands";
import { indentUnit, syntaxHighlighting } from "@codemirror/language";
import { StateEffect, StateField } from "@codemirror/state";
import { Decoration, EditorView, WidgetType, keymap } from "@codemirror/view";
import { cx, cxHighlightStyle, cxTheme } from "./cx-lang.js";
import { formatDiagnostic } from "./diagnostics.js";
import { initializeAllCodeEditors } from "./page.js";

const setDiagnosticsEffect = StateEffect.define();

const diagnosticsField = StateField.define({
    create: () => Decoration.none,
    update(decorations, transaction) {
        decorations = decorations.map(transaction.changes);
        for (const effect of transaction.effects) {
            if (effect.is(setDiagnosticsEffect)) decorations = effect.value;
        }
        return decorations;
    },
    provide: (field) => EditorView.decorations.from(field),
});

class DiagnosticWidget extends WidgetType {
    constructor(text, kind) {
        super();
        this.text = text;
        this.kind = kind;
    }

    toDOM() {
        const node = document.createElement("div");
        node.textContent = this.text;
        node.className = "diagnostic " + this.kind;
        return node;
    }
}

function createCM6Editor(wrapper, initialText) {
    const view = new EditorView({
        doc: initialText,
        extensions: [
            keymap.of([...defaultKeymap, ...historyKeymap, indentWithTab]),
            history(),
            indentUnit.of("    "),
            cx(),
            syntaxHighlighting(cxHighlightStyle),
            cxTheme,
            diagnosticsField,
        ],
        parent: wrapper,
    });
    return {
        getValue: () => view.state.doc.toString(),
        setValue: (code) => view.dispatch({ changes: { from: 0, to: view.state.doc.length, insert: code } }),
        setDiagnostics: (items) => {
            const decorations = items.map((item) => {
                const line = view.state.doc.line(Math.min(Math.max(item.line, 1), view.state.doc.lines));
                return Decoration.widget({
                    widget: new DiagnosticWidget(formatDiagnostic(item), item.kind),
                    block: true,
                    side: 1,
                }).range(line.to);
            });
            view.dispatch({ effects: setDiagnosticsEffect.of(Decoration.set(decorations, true)) });
        },
    };
}

document.addEventListener("DOMContentLoaded", () => initializeAllCodeEditors(createCM6Editor));
