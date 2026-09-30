// Tests website/playground-worker.js stage-1 paths (failed runs and the
// "check" action) with a stubbed compiler pipeline and toolchain loading.
// Run with: node wasm/test-playground-worker.mjs
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

// Canned stage-1 outcome; the "ok" mode returns generated C.
const stage1 = { mode: "error" };
const posted = [];
let fetchCalls = 0;

const sandbox = {
    console,
    importScripts: () => {},
    CxWasm: {},
    CxPipeline: {
        compileCxToC: async (factory, source) => {
            check(factory === sandbox.CxWasm, "worker passes the compiler factory through");
            check(source === "sent code", "worker passes the source through");
            if (stage1.mode === "ok") {
                return { status: 0, cCode: "int main(void) { return 0; }", diagnostics: { stdout: "", stderr: "" } };
            }
            const result = { status: 1, diagnostics: { stdout: "", stderr: "main.cx:1:1: error: boom\n" } };
            if (stage1.mode === "internal") {
                result.internalError = "kaboom";
            }
            return result;
        },
    },
    fetch: async () => {
        fetchCalls++;
        throw new Error("no toolchain in this test");
    },
    postMessage: (message) => {
        posted.push(message);
    },
    onmessage: null,
};
sandbox.globalThis = sandbox;
vm.createContext(sandbox);
vm.runInContext(await readFile(repoRoot + "website/playground-worker.js", "utf8"), sandbox);

async function send(message) {
    const before = posted.length;
    sandbox.onmessage({ data: message });
    for (let i = 0; i < 400 && posted.length === before; i++) {
        await new Promise((resolve) => setTimeout(resolve, 5));
    }
    return posted[posted.length - 1];
}

// "check" reports stage-1 diagnostics without touching the C toolchain.
stage1.mode = "error";
let response = await send({ action: "check", id: 1, code: "sent code" });
check(response.id === 1, "check answers with the request id");
check(response.stderr === "main.cx:1:1: error: boom\n", "check returns the compiler diagnostics");
check(fetchCalls === 0, "check loads no toolchain files");

// A failed "run" still reports stage-1 diagnostics (and only those).
response = await send({ action: "run", id: 2, code: "sent code" });
check(response.id === 2, "run answers with the request id");
check(response.stderr === "main.cx:1:1: error: boom\n", "failed run returns the compiler diagnostics");
check(response.stdout === "", "failed run returns no stdout");
check(fetchCalls === 0, "failed run loads no toolchain files");

// Internal compiler errors are appended to the diagnostics.
stage1.mode = "internal";
response = await send({ action: "run", id: 3, code: "sent code" });
check(
    response.stderr === "main.cx:1:1: error: boom\ninternal compiler error: kaboom\n",
    "failed run appends the internal error, got: " + JSON.stringify(response.stderr)
);

// A successful stage 1 proceeds to the C toolchain.
stage1.mode = "ok";
fetchCalls = 0;
response = await send({ action: "run", id: 4, code: "sent code" });
check(fetchCalls === 1, "successful stage 1 loads the C toolchain");
check(response.stderr.startsWith("error: "), "toolchain failure is reported, got: " + JSON.stringify(response.stderr));

if (failures > 0) {
    console.error(failures + " test(s) failed");
    process.exit(1);
}
console.log("All playground worker tests passed.");
