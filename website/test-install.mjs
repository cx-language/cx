// Exercises website/install.js OS-tab wiring with a minimal fake DOM.
// Run with: node website/test-install.mjs
import { readFile } from "node:fs/promises";
import vm from "node:vm";

const repoRoot = new URL("..", import.meta.url).pathname;
const src = await readFile(repoRoot + "website/install.js", "utf8");

let failures = 0;
function check(cond, message) {
    console.log((cond ? "PASS: " : "FAIL: ") + message);
    if (!cond) failures++;
}

function runCase(platform) {
    const pressed = {};
    const hidden = {};
    const buttons = ["unix", "windows"].map((os) => ({
        getAttribute: (k) => (k === "data-os" ? os : null),
        setAttribute: (k, v) => {
            if (k === "aria-pressed") pressed[os] = v;
        },
    }));
    const panes = ["unix", "windows"].map((os) => ({
        getAttribute: (k) => (k === "data-os" ? os : null),
        set hidden(v) {
            hidden[os] = v;
        },
    }));
    let clickFn = null;
    const tabs = {
        querySelectorAll: () => buttons,
        addEventListener: (name, fn) => {
            clickFn = fn;
        },
    };
    const listeners = {};
    const sandbox = {
        document: {
            querySelector: (sel) => (sel === ".os-tabs" ? tabs : null),
            querySelectorAll: (sel) => (sel === ".os-pane" ? panes : []),
            addEventListener: (name, fn) => {
                listeners[name] = fn;
            },
        },
        navigator: { platform, userAgentData: undefined },
    };
    vm.createContext(sandbox);
    vm.runInContext(src, sandbox);
    return {
        pressed,
        hidden,
        fireLoaded: () => listeners["DOMContentLoaded"](),
        click: (os) => clickFn({ target: { closest: () => buttons[os === "unix" ? 0 : 1] } }),
    };
}

let r = runCase("MacIntel");
check(Object.keys(r.pressed).length === 0, "nothing happens before DOMContentLoaded");
r.fireLoaded();
check(r.pressed.unix === "true" && r.hidden.windows === true, "mac defaults to unix pane");
r.click("windows");
check(r.pressed.windows === "true" && r.hidden.unix === true, "manual switch to windows pane");

r = runCase("Win32");
r.fireLoaded();
check(r.pressed.windows === "true" && r.hidden.unix === true, "windows detected");
r.click("unix");
check(r.pressed.unix === "true" && r.hidden.windows === true, "manual override back to unix");

if (failures > 0) {
    console.error(failures + " install test(s) failed");
    process.exit(1);
}
console.log("All install tests passed.");
