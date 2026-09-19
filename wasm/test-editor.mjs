// Exercises the docs playground editor wiring (docs/src/page.js) and
// diagnostic parsing (docs/src/diagnostics.js) with a minimal fake DOM.
// Run with: node wasm/test-editor.mjs
import { formatDiagnostic, parseDiagnostics } from "../docs/src/diagnostics.js";
import { initializeAllCodeEditors } from "../docs/src/page.js";

function makeElement(tag) {
    const el = {
        tagName: tag,
        children: [],
        style: {},
        attributes: {},
        className: "",
        innerText: "",
        disabled: false,
        blurred: false,
        parentNode: null,
        onclick: null,
        onchange: null,
        listeners: {},
        setAttribute(name, value) {
            el.attributes[name] = value;
        },
        addEventListener(name, fn) {
            (el.listeners[name] = el.listeners[name] || []).push(fn);
        },
        fireEvent(name) {
            (el.listeners[name] || []).forEach((fn) => fn());
        },
        blur() {
            el.blurred = true;
        },
        appendChild(child) {
            child.parentNode = el;
            el.children.push(child);
            return child;
        },
        replaceChild(nw, old) {
            const i = el.children.indexOf(old);
            el.children[i] = nw;
            nw.parentNode = el;
            old.parentNode = null;
            return old;
        },
        querySelector(sel) {
            const all = [];
            const walk = (node) => node.children.forEach((c) => {
                all.push(c);
                walk(c);
            });
            walk(el);
            if (sel === "#example-selector") return all.find((c) => c.attrsId === "example-selector") || null;
            if (sel === "select") return all.find((c) => c.tagName === "select") || null;
            return null;
        },
        querySelectorAll() {
            return [];
        },
        closest(sel) {
            const dot = sel.indexOf(".");
            const tag = dot < 0 ? sel : sel.slice(0, dot);
            const cls = dot < 0 ? null : sel.slice(dot + 1);
            let node = el;
            while (node) {
                const tagOk = !tag || node.tagName === tag;
                const clsOk = !cls || (" " + (node.className || "") + " ").includes(" " + cls + " ");
                if (tagOk && clsOk) return node;
                node = node.parentNode;
            }
            return null;
        },
        scrollIntoView() {},
        click() {
            el.onclick && el.onclick();
        },
        attrsId: null,
    };
    return el;
}

let failures = 0;
function check(condition, message) {
    if (!condition) {
        console.error("FAIL: " + message);
        failures++;
    } else {
        console.log("PASS: " + message);
    }
}

function checkEqual(actual, expected, message) {
    check(JSON.stringify(actual) === JSON.stringify(expected),
        message + " (got " + JSON.stringify(actual) + ", want " + JSON.stringify(expected) + ")");
}

// Fake editor, standing in for the CodeMirror 6 adapter.
let editorValue = "";
const diagnosticCalls = [];
const createdEditors = [];
function createEditor(wrapper, initialText) {
    editorValue = initialText;
    createdEditors.push(wrapper);
    return {
        getValue() {
            return editorValue;
        },
        setValue(v) {
            editorValue = v;
        },
        setDiagnostics(items) {
            diagnosticCalls.push(items);
        },
    };
}

// Build: <div class="showcase"><label><select id=...>[2 options]</select></label><pre .../></div>
const showcase = makeElement("div");
showcase.className = "showcase";
const label = makeElement("label");
const selector = makeElement("select");
selector.attrsId = "example-selector";
selector.value = "0";
label.appendChild(selector);
showcase.appendChild(label);
const block = makeElement("pre");
block.className = "sourceCode";
block.innerText = "old code";
showcase.appendChild(block);

const ranWith = [];
let runResult = async (code) => {
    ranWith.push(code);
    return { stdout: "ran: " + code, stderr: "" };
};
globalThis.CxExamples = [
    { name: "Prime sieve", code: "sieve code" },
    { name: "Hello world", code: "hello code" },
];
globalThis.CxPlayground = {
    warmUp() {},
    isSupported: () => true,
    run: (code) => runResult(code),
};
globalThis.document = {
    querySelectorAll: () => [block],
    createElement: makeElement,
};
// Keep run timers out of the event loop; the promise assertions below
// don't depend on the "Running..." ticker.
globalThis.setInterval = () => 0;
globalThis.clearInterval = () => {};

initializeAllCodeEditors(createEditor);
await new Promise((resolve) => setTimeout(resolve, 10));

check(editorValue === "old code", "editor initialized from block content");
check(createdEditors.length === 1 && createdEditors[0].className === "editor", "editor hosted in a div.editor");
check(block.parentNode === null, "editor replaces the code block");

const runButton = showcase.children.find((c) => c.className === "run");
const output = showcase.children.find((c) => c.className === "output");
const stdout = output.children.find((c) => c.className === "stdout");
const stderr = output.children.find((c) => c.className === "stderr");
// Fake innerText concatenation across the stdout/stderr divs.
Object.defineProperty(output, "innerText", {
    get: () => [stdout.innerText, stderr.innerText].filter((t) => t !== "").join("\n"),
});

check(runButton && runButton.attributes["aria-label"] === "Run", "run button has an accessible label");

selector.value = "1";
selector.fireEvent("pointerdown");
selector.onchange();
await new Promise((resolve) => setTimeout(resolve, 10));

check(editorValue === "hello code", "switching selector sets editor content");
check(ranWith.length === 1 && ranWith[0] === "hello code", "switching auto-runs the new example, got: " + JSON.stringify(ranWith));
check(selector.blurred, "focus is dropped after pointer selection");
checkEqual(diagnosticCalls.at(-1), [], "switching examples clears diagnostics");

selector.blurred = false;
selector.value = "0";
selector.fireEvent("keydown");
selector.onchange();
await new Promise((resolve) => setTimeout(resolve, 10));

check(editorValue === "sieve code", "keyboard switching sets editor content");
check(selector.blurred === false, "focus is kept after keyboard selection");

// Diagnostics from a failing run are parsed and shown in the editor.
runResult = async (code) => {
    ranWith.push(code);
    return { stdout: "", stderr: "main.cx:2:5: use of undeclared identifier 'x'\n    x + 1;\n        ^\n" };
};
runButton.click();
await new Promise((resolve) => setTimeout(resolve, 10));

checkEqual(diagnosticCalls.at(-1), [{
    line: 2,
    column: 5,
    message: "use of undeclared identifier 'x'",
    indent: "        ",
    kind: "error",
}], "run errors are parsed and shown as diagnostics");
checkEqual(formatDiagnostic(diagnosticCalls.at(-1)[0]), "        ^ use of undeclared identifier 'x'",
    "diagnostics render caret plus message");

// Unit checks for the diagnostic parser.
checkEqual(parseDiagnostics("hello\n"), [], "clean output yields no diagnostics");
checkEqual(parseDiagnostics("main.cx:1:1: warning: unused variable 'x'\n"), [{
    line: 1, column: 1, message: "warning: unused variable 'x'", indent: "", kind: "warning",
}], "warnings are classified");
checkEqual(parseDiagnostics("main.cx:1:1: first\nmain.cx:3:7: second\n").map((d) => d.line), [1, 3],
    "multiple diagnostics are all reported");

if (failures > 0) {
    console.error(failures + " test(s) failed");
    process.exit(1);
}
console.log("All editor tests passed.");
