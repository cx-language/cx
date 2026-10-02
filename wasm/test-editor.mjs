// Exercises website/editor.js showcase wiring with a minimal fake DOM.
// Run with: node wasm/test-editor.mjs
import { readFile } from "node:fs/promises";
import vm from "node:vm";

const repoRoot = new URL("..", import.meta.url).pathname;

function makeElement(tag) {
    const el = {
        tagName: tag,
        children: [],
        style: {},
        className: "",
        classList: {
            add(...names) {
                el.className = (el.className + " " + names.join(" ")).trim();
            },
            contains(name) {
                return (" " + el.className + " ").includes(" " + name + " ");
            },
        },
        _text: "",
        disabled: false,
        blurred: false,
        parentNode: null,
        onclick: null,
        onmessage: null,
        listeners: {},
        addEventListener(name, fn) {
            (el.listeners[name] = el.listeners[name] || []).push(fn);
        },
        fireEvent(name, arg) {
            (el.listeners[name] || []).forEach((fn) => fn(arg));
        },
        appendChild(child) {
            child.parentNode = el;
            el.children.push(child);
            return child;
        },
        insertBefore(child, ref) {
            child.parentNode = el;
            const i = ref ? el.children.indexOf(ref) : -1;
            if (i < 0) el.children.push(child);
            else el.children.splice(i, 0, child);
            return child;
        },
        replaceChild(nw, old) {
            const i = el.children.indexOf(old);
            el.children[i] = nw;
            nw.parentNode = el;
            old.parentNode = null;
            return old;
        },
        replaceWith(nw) {
            el.parentNode.replaceChild(nw, el);
        },
        querySelector(sel) {
            return el.querySelectorAll(sel)[0] || null;
        },
        querySelectorAll(sel) {
            const all = [];
            const walk = (node) => node.children.forEach((c) => {
                all.push(c);
                walk(c);
            });
            walk(el);
            if (!sel) return [];
            if (sel[0] === ".") return all.filter((c) => c.classList.contains(sel.slice(1)));
            return all.filter((c) => c.tagName === sel);
        },
        closest(sel) {
            const dot = sel.indexOf(".");
            const tag = dot < 0 ? sel : sel.slice(0, dot);
            const cls = dot < 0 ? null : sel.slice(dot + 1);
            let node = el;
            while (node) {
                const tagOk = !tag || node.tagName === tag;
                const clsOk = !cls || node.classList.contains(cls);
                if (tagOk && clsOk) return node;
                node = node.parentNode;
            }
            return null;
        },
        scrollIntoView() {},
        click() {
            el.onclick && el.onclick();
        },
        attrs: {},
        setAttribute(name, value) {
            el.attrs[name] = value;
        },
        getAttribute(name) {
            return el.attrs[name];
        },
    };
    // innerText renders the element's children like the real DOM, so the
    // output container reads back the concatenated stdout/stderr divs.
    Object.defineProperty(el, "innerText", {
        get() {
            return el.children.map((c) => c.innerText).join("") + el._text;
        },
        set(v) {
            el.children = [];
            el._text = v;
        },
    });
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

// Fake CodeMirror.
let editorValue = "";
const widgets = [];
const editorListeners = {};
const fakeEditor = {
    refresh() {},
    getValue() {
        return editorValue;
    },
    setValue(v) {
        editorValue = v;
    },
    on(name, fn) {
        editorListeners[name] = fn;
    },
    addLineWidget(line, node) {
        const widget = { line, text: node.innerText, node };
        widgets.push(widget);
        return {
            clear() {
                widgets.splice(widgets.indexOf(widget), 1);
            },
        };
    },
};

function FakeCodeMirror(wrapper, opts) {
    cmCalls.push({ wrapper, opts });
    if (typeof wrapper === "function") wrapper(makeElement("div"));
    editorValue = opts.value;
    return fakeEditor;
}
FakeCodeMirror.registerHelper = (type, mode, fn) => {
    registeredHelpers[type + ":" + mode] = fn;
};
FakeCodeMirror.Pos = (line, ch) => ({ line, ch });

// Build: <div class="showcase"><div class="example-tabs">[2 buttons]</div><div class="sourceCode"><pre .../></div></div>
// (pandoc wraps showcase code in div.sourceCode in production; mirror that).
const showcase = makeElement("div");
showcase.className = "showcase";
const tabs = makeElement("div");
tabs.className = "example-tabs";
function makeTab(index, selected) {
    const button = makeElement("button");
    button.setAttribute("data-tab", String(index));
    button.setAttribute("aria-pressed", selected ? "true" : "false");
    tabs.appendChild(button);
    return button;
}
const tab0 = makeTab(0, true);
const tab1 = makeTab(1, false);
const moreLink = makeElement("a");
moreLink.className = "more";
tabs.appendChild(moreLink);
showcase.appendChild(tabs);
const codeWrap = makeElement("div");
codeWrap.className = "sourceCode";
const block = makeElement("pre");
block.className = "sourceCode";
block.innerText = "old code";
codeWrap.appendChild(block);
showcase.appendChild(codeWrap);

const snippetWrap = makeElement("div");
const snippetPre = makeElement("pre");
snippetPre.className = "snippet cx";
snippetPre.innerText = "int? x = null;";
snippetWrap.appendChild(snippetPre);

const ranWith = [];
const cmCalls = [];
let scriptedResult = null;
const checkedWith = [];
let scriptedCheck = { stdout: "", stderr: "" };
let timers = [];
const sandbox = {
    console,
    setTimeout: (fn) => {
        timers.push(fn);
        return timers.length;
    },
    clearTimeout: (id) => {
        if (id) timers[id - 1] = null;
    },
    setInterval: () => 0,
    clearInterval: () => {},
    CodeMirror: FakeCodeMirror,
    CxExamples: [
        { name: "Prime sieve", code: "sieve code" },
        { name: "Hello world", code: "hello code" },
    ],
    CxPlayground: {
        warmUp() {},
        isSupported: () => true,
        run: async (code) => {
            ranWith.push(code);
            return scriptedResult || { stdout: "ran: " + code, stderr: "" };
        },
        check: async (code) => {
            checkedWith.push(code);
            return typeof scriptedCheck === "function" ? scriptedCheck(code) : scriptedCheck;
        },
    },
    document: {
        listeners: {},
        addEventListener(name, fn) {
            sandbox.document.listeners[name] = fn;
        },
        querySelectorAll(sel) {
            if (sel === "pre.snippet.cx") return [snippetPre];
            return [block];
        },
        createElement: makeElement,
        createTextNode(text) {
            const node = makeElement("#text");
            node.innerText = text;
            return node;
        },
    },
};
sandbox.globalThis = sandbox;
vm.createContext(sandbox);
vm.runInContext(await readFile(repoRoot + "website/editor.js", "utf8"), sandbox);

// Simulate page load.
sandbox.document.listeners["DOMContentLoaded"]();
await new Promise((resolve) => setTimeout(resolve, 10));

check(editorValue === "old code", "editor initialized from block content");
check(
    cmCalls.some((c) => typeof c.wrapper === "function" && c.opts.value === "int? x = null;" && c.opts.readOnly === "nocursor"),
    "card snippets get a read-only highlight pass"
);
check(
    snippetWrap.children.length === 1 && snippetWrap.children[0].classList.contains("snippet"),
    "highlight pass swaps the pre for a snippet box"
);

const runButton = showcase.querySelector(".run");
check(runButton && runButton.parentNode === codeWrap, "run button lands in the code wrapper");
check(
    codeWrap.querySelector(".output") !== null,
    "run output lands in the code wrapper"
);

tabs.fireEvent("click", { target: tab1 });
await new Promise((resolve) => setTimeout(resolve, 10));

check(editorValue === "hello code", "switching tabs sets editor content");
check(ranWith.length === 1 && ranWith[0] === "hello code", "switching auto-runs the new example, got: " + JSON.stringify(ranWith));
check(
    tab0.getAttribute("aria-pressed") === "false" && tab1.getAttribute("aria-pressed") === "true",
    "aria-pressed follows the active tab"
);

tabs.fireEvent("click", { target: tab0 });
await new Promise((resolve) => setTimeout(resolve, 10));

check(editorValue === "sieve code", "switching back sets editor content");

tabs.fireEvent("click", { target: tabs });
await new Promise((resolve) => setTimeout(resolve, 10));

check(editorValue === "sieve code", "clicking between tabs keeps editor content");
check(ranWith.length === 2, "clicking between tabs runs nothing, got: " + JSON.stringify(ranWith));

tabs.fireEvent("click", { target: moreLink });
await new Promise((resolve) => setTimeout(resolve, 10));

check(editorValue === "sieve code", "more link keeps editor content");
check(ranWith.length === 2, "more link runs nothing, got: " + JSON.stringify(ranWith));

// Diagnostic widgets: the compiler underlines ranges with '~' and points
// with '^'; both must render without an "undefined" prefix.
async function runWithStderr(stderr) {
    widgets.length = 0;
    scriptedResult = { stdout: "", stderr };
    runButton.click();
    await new Promise((resolve) => setTimeout(resolve, 10));
    scriptedResult = null;
}

await runWithStderr(
    "main.cx:1:42: error: unknown identifier 'isEven'\n" +
        "void main() { var d = List<int>().filter(isEven); }\n" +
        "                                         ~~~~~~\n"
);
check(widgets.length === 1, "range diagnostic produces one widget");
check(widgets[0].line === 0, "widget is placed on the error line (0-based)");
check(
    widgets[0].text === " ".repeat(41) + "^ unknown identifier 'isEven'",
    "tilde-underlined diagnostic keeps its indent, got: " + JSON.stringify(widgets[0].text)
);
check(!widgets[0].text.includes("undefined"), "widget text has no undefined prefix");
check(widgets[0].node.classList.contains("error"), "error diagnostic gets the error class");

await runWithStderr("main.cx:2:5: warning: unused variable 'x'\n    var x = 1;\n        ^\n");
check(widgets.length === 1, "caret diagnostic produces one widget");
check(widgets[0].text === "        ^ unused variable 'x'", "caret diagnostic keeps its indent, got: " + JSON.stringify(widgets[0].text));
check(widgets[0].node.classList.contains("warning"), "warning diagnostic gets the warning class");

await runWithStderr("main.cx:1:1: error: something broke\n");
check(widgets.length === 1, "context-less diagnostic produces one widget");
check(widgets[0].text === "^ something broke", "context-less diagnostic has no undefined prefix, got: " + JSON.stringify(widgets[0].text));

await runWithStderr(
    "main.cx:1:5: error: unknown identifier 'x'\n" +
        "var y = x;\n" +
        "    ~\n" +
        "main.cx:1:6: note: 'foo' declared here\n" +
        "void foo() {}\n" +
        "     ^\n" +
        "main.cx:2:6: warning: unused declaration 'bar'\n" +
        "void bar() {}\n" +
        "     ~~~\n"
);
check(widgets.length === 1, "notes and unused-declaration warnings stay out of the editor");
check(
    widgets[0].text === " ".repeat(4) + "^ unknown identifier 'x'",
    "only the error gets a widget, got: " + JSON.stringify(widgets.map((w) => w.text))
);

// Live diagnostics: editing triggers a background check whose errors show
// without pressing Run.
async function flushTimers() {
    const pending = timers;
    timers = [];
    pending.forEach((fn) => fn && fn());
    await new Promise((resolve) => setTimeout(resolve, 10));
}

check(typeof editorListeners.change === "function", "editor subscribes to change events");

widgets.length = 0;
checkedWith.length = 0;
scriptedCheck = { stdout: "", stderr: "main.cx:1:1: error: something broke\n" };
editorListeners.change();
await flushTimers();
check(checkedWith.length === 1 && checkedWith[0] === editorValue, "change triggers a check of the current code");
check(
    widgets.length === 1 && widgets[0].text === "^ something broke",
    "check errors show without running, got: " + JSON.stringify(widgets.map((w) => w.text))
);

checkedWith.length = 0;
scriptedCheck = { stdout: "", stderr: "" };
editorListeners.change();
editorListeners.change();
await flushTimers();
check(checkedWith.length === 1, "debounced changes trigger a single check");
check(widgets.length === 0, "clean check clears the widgets");

// A check resolving after a Run started is discarded.
let resolveCheck;
scriptedCheck = new Promise((resolve) => {
    resolveCheck = resolve;
});
editorListeners.change();
await flushTimers();
scriptedResult = { stdout: "ok", stderr: "" };
runButton.click();
resolveCheck({ stdout: "", stderr: "main.cx:1:1: error: stale\n" });
await new Promise((resolve) => setTimeout(resolve, 10));
scriptedResult = null;
check(widgets.length === 0, "stale check does not clobber run widgets, got: " + JSON.stringify(widgets.map((w) => w.text)));

// Overlapping checks: the first resolving last is discarded.
widgets.length = 0;
let resolveFirst;
let first = true;
scriptedCheck = () => {
    if (first) {
        first = false;
        return new Promise((resolve) => {
            resolveFirst = resolve;
        });
    }
    return { stdout: "", stderr: "main.cx:2:1: warning: second\n" };
};
editorListeners.change();
await flushTimers();
editorListeners.change();
await flushTimers();
resolveFirst({ stdout: "", stderr: "main.cx:1:1: error: first\n" });
await new Promise((resolve) => setTimeout(resolve, 10));
scriptedCheck = { stdout: "", stderr: "" };
check(
    widgets.length === 1 && widgets[0].text === "^ second",
    "late first check is discarded, got: " + JSON.stringify(widgets.map((w) => w.text))
);

// A check applying mid-run does not duplicate the run's widgets.
widgets.length = 0;
let resolveRun;
scriptedCheck = { stdout: "", stderr: "main.cx:1:1: error: live\n" };
scriptedResult = new Promise((resolve) => {
    resolveRun = resolve;
});
runButton.click();
editorListeners.change();
await flushTimers();
check(widgets.length === 1, "check applies while a run is in flight");
resolveRun({ stdout: "", stderr: "main.cx:1:1: error: live\n" });
await new Promise((resolve) => setTimeout(resolve, 10));
scriptedResult = null;
scriptedCheck = { stdout: "", stderr: "" };
check(widgets.length === 1, "run completion replaces mid-flight check widgets, got: " + JSON.stringify(widgets.map((w) => w.text)));

// A Run between the change and the debounce firing skips the check.
widgets.length = 0;
checkedWith.length = 0;
editorListeners.change();
scriptedResult = { stdout: "", stderr: "" };
runButton.click();
await flushTimers();
scriptedResult = null;
check(checkedWith.length === 0, "run between change and fire skips the check");

if (failures > 0) {
    console.error(failures + " test(s) failed");
    process.exit(1);
}
console.log("All editor tests passed.");
