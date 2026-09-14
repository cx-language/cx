// Exercises docs/editor.js showcase wiring with a minimal fake DOM.
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
        innerText: "",
        disabled: false,
        blurred: false,
        parentNode: null,
        onclick: null,
        onchange: null,
        onmessage: null,
        listeners: {},
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

// Fake CodeMirror.
let editorValue = "";
const fakeEditor = {
    refresh() {},
    getValue() {
        return editorValue;
    },
    setValue(v) {
        editorValue = v;
    },
    addLineWidget() {
        return { clear() {} };
    },
};

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
const sandbox = {
    console,
    setTimeout: (fn) => 0,
    setInterval: () => 0,
    clearInterval: () => {},
    CodeMirror: (wrapper, opts) => {
        editorValue = opts.value;
        return fakeEditor;
    },
    CxExamples: [
        { name: "Prime sieve", code: "sieve code" },
        { name: "Hello world", code: "hello code" },
    ],
    CxPlayground: {
        warmUp() {},
        isSupported: () => true,
        run: async (code) => {
            ranWith.push(code);
            return { stdout: "ran: " + code, stderr: "" };
        },
    },
    document: {
        listeners: {},
        addEventListener(name, fn) {
            sandbox.document.listeners[name] = fn;
        },
        querySelectorAll() {
            return [block];
        },
        createElement: makeElement,
    },
};
sandbox.globalThis = sandbox;
vm.createContext(sandbox);
vm.runInContext(await readFile(repoRoot + "docs/editor.js", "utf8"), sandbox);

// Simulate page load.
sandbox.document.listeners["DOMContentLoaded"]();
await new Promise((resolve) => setTimeout(resolve, 10));

check(editorValue === "old code", "editor initialized from block content");

selector.value = "1";
selector.fireEvent("pointerdown");
selector.onchange();
await new Promise((resolve) => setTimeout(resolve, 10));

check(editorValue === "hello code", "switching selector sets editor content");
check(ranWith.length === 1 && ranWith[0] === "hello code", "switching auto-runs the new example, got: " + JSON.stringify(ranWith));
check(selector.blurred, "focus is dropped after pointer selection");

selector.blurred = false;
selector.value = "0";
selector.fireEvent("keydown");
selector.onchange();
await new Promise((resolve) => setTimeout(resolve, 10));

check(editorValue === "sieve code", "keyboard switching sets editor content");
check(selector.blurred === false, "focus is kept after keyboard selection");

if (failures > 0) {
    console.error(failures + " test(s) failed");
    process.exit(1);
}
console.log("All editor tests passed.");
