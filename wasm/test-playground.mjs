// Tests website/playground.js timeout behavior with a stubbed Worker and
// controllable timers: a hung run terminates the worker (stopping infinite
// user loops), while a hung background check never does, so a slow check
// can't kill a foreground run. Run with: node wasm/test-playground.mjs
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

const timers = [];
const workers = [];
class FakeWorker {
    constructor(url) {
        this.url = url;
        this.sent = [];
        this.terminated = false;
        this.onmessage = null;
        this.onerror = null;
        workers.push(this);
    }
    postMessage(message) {
        this.sent.push(message);
    }
    terminate() {
        this.terminated = true;
    }
}

const sandbox = {
    console,
    Worker: FakeWorker,
    WebAssembly: { instantiate: () => {} },
    BigInt,
    setTimeout: (fn) => {
        timers.push(fn);
        return timers.length;
    },
    clearTimeout: (id) => {
        if (id) timers[id - 1] = null;
    },
};
sandbox.globalThis = sandbox;
vm.createContext(sandbox);
vm.runInContext(await readFile(repoRoot + "website/playground.js", "utf8"), sandbox);
const CxPlayground = sandbox.CxPlayground;

check(CxPlayground.isSupported(), "playground is supported with Worker, WebAssembly, and BigInt");

function fireTimer(index) {
    const fn = timers[index];
    timers[index] = null;
    fn();
}

// A hung run terminates the worker.
const runOutcome = CxPlayground.run("code");
check(workers.length === 1, "run creates a worker");
check(workers[0].sent.length === 1 && workers[0].sent[0].action === "run", "run posts a run action");
fireTimer(0);
const runResult = await runOutcome;
check(workers[0].terminated, "timed-out run terminates the worker");
check(runResult.stderr.includes("infinite loop"), "timed-out run reports a possible infinite loop, got: " + JSON.stringify(runResult.stderr));

// A hung check leaves the worker alone.
const foreground = CxPlayground.run("code");
const background = CxPlayground.check("code");
check(workers.length === 2, "run after termination creates a fresh worker");
check(workers[1].sent.length === 2 && workers[1].sent[1].action === "check", "check posts a check action");
fireTimer(timers.length - 1);
const checkResult = await background;
check(checkResult.stdout === "" && checkResult.stderr === "", "timed-out check resolves to empty diagnostics");
check(!workers[1].terminated, "timed-out check does not terminate the worker");
check(workers[1].sent[0].action === "run", "foreground run is still pending on the live worker");
// The foreground run stays unsettled; that is the point (its worker lives).

// A hung completion behaves like a hung check: no items, worker lives.
const completion = CxPlayground.complete("code", 3, 7);
check(workers[1].sent[workers[1].sent.length - 1].action === "complete", "complete posts a complete action");
check(workers[1].sent[workers[1].sent.length - 1].line === 3, "complete posts the cursor position");
fireTimer(timers.length - 1);
const completionResult = await completion;
check(Array.isArray(completionResult.items) && completionResult.items.length === 0, "timed-out complete resolves to no items");
check(completionResult.failed === true, "timed-out complete is flagged failed");
check(!workers[1].terminated, "timed-out complete does not terminate the worker");

// An answered completion resolves with the items and no failed flag.
const answered = CxPlayground.complete("code", 0, 0);
const answeredSent = workers[1].sent[workers[1].sent.length - 1];
workers[1].onmessage({ data: { id: answeredSent.id, items: [{ label: "x" }] } });
const answeredResult = await answered;
check(answeredResult.items.length === 1 && answeredResult.items[0].label === "x", "answered complete resolves the items");
check(answeredResult.failed === false, "answered complete is not flagged failed");

if (failures > 0) {
    console.error(failures + " test(s) failed");
    process.exit(1);
}
console.log("All playground tests passed.");
