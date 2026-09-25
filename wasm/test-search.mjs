// Exercises website/search.js with a minimal fake DOM.
// Run with: node wasm/test-search.mjs
import { readFile } from "node:fs/promises";
import vm from "node:vm";

const repoRoot = new URL("..", import.meta.url).pathname;

let failures = 0;
function check(condition, message) {
    if (!condition) {
        console.error("FAIL: " + message);
        failures++;
    } else {
        console.log("PASS: " + message);
    }
}

function makeElement(tag) {
    const el = {
        tagName: tag,
        children: [],
        className: "",
        hidden: false,
        value: "",
        _text: "",
        attrs: {},
        listeners: {},
        addEventListener(name, fn) {
            (el.listeners[name] = el.listeners[name] || []).push(fn);
        },
        fire(name, event) {
            (el.listeners[name] || []).forEach((fn) => fn(event || {}));
        },
        appendChild(child) {
            child.parentNode = el;
            el.children.push(child);
            return child;
        },
        closest(sel) {
            const cls = sel.startsWith(".") ? sel.slice(1) : null;
            let node = el;
            while (node) {
                if (cls && (" " + (node.className || "") + " ").includes(" " + cls + " ")) return node;
                node = node.parentNode;
            }
            return null;
        },
        getAttribute(name) {
            return el.attrs[name];
        },
        setAttribute(name, value) {
            el.attrs[name] = value;
        },
    };
    Object.defineProperty(el, "textContent", {
        get() {
            return el.children.map((c) => c.textContent).join("") + el._text;
        },
        set(v) {
            el.children = [];
            el._text = v;
        },
    });
    Object.defineProperty(el, "innerHTML", {
        get() {
            return "";
        },
        set() {
            el.children = [];
            el._text = "";
        },
    });
    return el;
}

const PAGES = [
    { id: "introduction", title: "Introduction", text: "cx is a language for performance and productivity" },
    { id: "list", title: "List", text: "The List type holds elements. Use map and filter on lists." },
    { id: "std/Map", title: "Map", text: "Associative container mapping keys to values." },
];

async function makeHarness(locationHref, homeHref, scriptSrc) {
    const timers = [];
    const input = makeElement("input");
    const box = makeElement("div");
    const container = makeElement("div");
    container.className = "search";
    container.appendChild(input);
    container.appendChild(box);
    const home = makeElement("a");
    home.setAttribute("href", homeHref);
    const scriptTag = makeElement("script");
    scriptTag.setAttribute("src", scriptSrc);
    const headScripts = [];
    const head = makeElement("head");
    const append = head.appendChild.bind(head);
    head.appendChild = (child) => {
        headScripts.push(child);
        return append(child);
    };
    const window = { location: { href: locationHref } };
    const sandbox = {
        console,
        URL,
        setTimeout: (fn) => {
            timers.push(fn);
            return timers.length;
        },
        clearTimeout: (id) => {
            if (id) timers[id - 1] = null;
        },
        window,
        document: {
            listeners: {},
            addEventListener(name, fn) {
                sandbox.document.listeners[name] = fn;
            },
            getElementById(id) {
                if (id === "site-search") return input;
                if (id === "site-search-results") return box;
                return null;
            },
            querySelector(sel) {
                if (sel === ".top-nav h1 a") return home;
                if (sel === 'script[src$="search.js"]') return scriptTag;
                return null;
            },
            createElement: makeElement,
            createTextNode(text) {
                const node = makeElement("#text");
                node.textContent = text;
                return node;
            },
            head,
        },
    };
    sandbox.globalThis = sandbox;
    vm.createContext(sandbox);
    vm.runInContext(await readFile(repoRoot + "website/search.js", "utf8"), sandbox);
    sandbox.document.listeners["DOMContentLoaded"]();

    const harness = {
        sandbox,
        input,
        box,
        headScripts,
        window,
        flush() {
            timers.splice(0).forEach((fn) => fn && fn());
        },
        type(text) {
            input.value = text;
            input.fire("input");
            harness.flush();
        },
        loadIndex() {
            sandbox.CxSearchIndex = PAGES;
            headScripts[headScripts.length - 1].onload();
        },
        resultLinks() {
            return box.children.filter((c) => c.tagName === "a");
        },
        click(target) {
            sandbox.document.listeners["click"]({ target });
        },
    };
    return harness;
}

// Top-level page: lazy load, ranking, snippets, keyboard.
const top = await makeHarness("https://example.com/introduction.html", "./", "search.js");
check(top.headScripts.length === 0, "index is not loaded before first input");
top.type("map");
check(top.headScripts.length === 1, "first input injects the index script");
check(top.headScripts[0].src === "search-index.js", "index URL sits next to search.js");
top.loadIndex();
let links = top.resultLinks();
check(
    links.length === 2 && links[0].href === "https://example.com/std/Map" && links[1].href === "https://example.com/list",
    "title match ranks before body match, got: " + JSON.stringify(links.map((l) => l.href))
);
check(top.box.hidden === false, "results are shown");

top.type("list filter");
links = top.resultLinks();
check(links.length === 1 && links[0].href === "https://example.com/list", "multi-term query matches pages containing every term");
const snip = links[0].children.find((c) => c.className === "result-snippet");
const mark = snip.children.find((c) => c.tagName === "mark");
check(mark && mark.textContent === "List", "snippet highlights the first match");

top.type("zzz");
check(top.resultLinks().length === 0 && top.box.textContent === "No results.", "no match shows an empty note");

top.type("");
check(top.box.hidden === true, "empty query hides the results");
check(top.headScripts.length === 1, "index loads only once");

top.type("map");
top.input.fire("keydown", { key: "Escape" });
check(top.input.value === "" && top.box.hidden === true, "Escape clears and hides");

top.type("map");
top.input.fire("keydown", { key: "Enter" });
check(top.window.location.href === "https://example.com/std/Map", "Enter navigates to the first hit");

// Stale hits never navigate.
top.type("map");
top.input.fire("keydown", { key: "Escape" });
top.window.location.href = "https://example.com/introduction.html";
top.input.fire("keydown", { key: "Enter" });
check(top.window.location.href === "https://example.com/introduction.html", "Enter after Escape does not navigate");

top.type("map");
top.type("");
top.input.fire("keydown", { key: "Enter" });
check(top.window.location.href === "https://example.com/introduction.html", "Enter on empty query does not navigate");

// Click-outside dismisses; clicks inside keep results open.
top.type("map");
top.click(makeElement("p"));
check(top.box.hidden === true, "outside click hides the results");
top.type("map");
top.click(top.resultLinks()[0]);
check(top.box.hidden === false, "result click keeps the results open");

// Refocusing reopens dismissed results, but not cleared ones.
top.click(makeElement("p"));
top.input.fire("focus");
check(top.box.hidden === false && top.resultLinks().length === 2, "focus reopens dismissed results");
top.input.fire("keydown", { key: "Escape" });
top.input.fire("focus");
check(top.box.hidden === true, "focus after Escape stays hidden");
top.type("map");
top.type("");
top.input.fire("focus");
check(top.box.hidden === true, "focus after clearing stays hidden");

// Rapid input before the index arrives queues behind one load.
const queued = await makeHarness("https://example.com/introduction.html", "./", "search.js");
queued.input.value = "li";
queued.input.fire("input");
queued.input.value = "list";
queued.input.fire("input");
queued.flush();
check(queued.headScripts.length === 1, "overlapping inputs share one index load");
queued.loadIndex();
check(queued.resultLinks().length === 1, "queued render uses the latest query");

// Failed index load degrades to a note, tried once.
const broken = await makeHarness("https://example.com/introduction.html", "./", "search.js");
broken.type("map");
broken.headScripts[0].onerror();
check(broken.box.hidden === false && broken.box.textContent === "Search is unavailable.", "failed load shows a note");
broken.type("map filter");
check(broken.headScripts.length === 1, "failed load is not retried");

// Nested page: index URL and result links resolve to the site root.
const nested = await makeHarness("https://example.com/cx/std/Array.html", "../", "../search.js");
nested.type("map");
check(nested.headScripts[0].src === "../search-index.js", "nested page loads the index relatively");
nested.loadIndex();
links = nested.resultLinks();
check(links.length === 2 && links[0].href === "https://example.com/cx/std/Map", "nested result links resolve to the site root, got: " + JSON.stringify(links.map((l) => l.href)));

if (failures > 0) {
    console.error(failures + " test(s) failed");
    process.exit(1);
}
console.log("All search tests passed.");
