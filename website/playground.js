// Main-thread interface to the cx playground Web Worker.
//
// The worker compiles and runs cx code entirely in the browser: the cx
// compiler frontend (compiled to WebAssembly) translates the program to C,
// an in-browser C compiler translates it to WebAssembly, and the result is
// executed with captured stdout/stderr. See playground-worker.js.
//
// Usage: CxPlayground.run(code) returns a promise of {stdout, stderr}.
// CxPlayground.check(code) returns a promise of {stdout, stderr} with the
// stage-1 diagnostics only, for live error display while editing.
(function (global) {
    "use strict";

    var WORKER_URL = "playground-worker.js";
    var WARMUP_TIMEOUT_MS = 120000;
    var RUN_TIMEOUT_MS = 60000;
    var CHECK_TIMEOUT_MS = 60000;

    var worker = null;
    var nextId = 0;
    var pending = new Map();

    // Whether this browser can run the playground at all: it needs Web
    // Workers, WebAssembly (including BigInt integration for 64-bit imports),
    // and fetch/Promise, all of which are missing together on old browsers.
    function isSupported() {
        return typeof Worker !== "undefined" &&
            typeof WebAssembly !== "undefined" &&
            typeof WebAssembly.instantiate === "function" &&
            typeof BigInt !== "undefined";
    }

    function unsupportedMessage() {
        return "error: the online playground needs WebAssembly, which your browser doesn't support. " +
            "Please try a recent version of Chrome, Firefox, Safari, or Edge.\n";
    }

    function ensureWorker() {
        if (!worker) {
            worker = new Worker(WORKER_URL);
            worker.onmessage = function (event) {
                var message = event.data;
                var entry = pending.get(message.id);
                if (entry) {
                    pending.delete(message.id);
                    clearTimeout(entry.timer);
                    entry.resolve(message);
                }
            };
            worker.onerror = function (event) {
                var error = new Error("playground worker failed: " + (event.message || "unknown error"));
                pending.forEach(function (entry) {
                    clearTimeout(entry.timer);
                    entry.reject(error);
                });
                pending.clear();
                worker.terminate();
                worker = null;
            };
        }
        return worker;
    }

    function postMessage(message, timeoutMs, terminateOnTimeout) {
        return new Promise(function (resolve, reject) {
            var id = ++nextId;
            message.id = id;
            var timer = setTimeout(function () {
                pending.delete(id);
                // Runs terminate the worker to also stop infinite loops in
                // user programs; warmup and check timeouts abandon it instead
                // so that slow networks still get a chance (and a slow
                // background check can't kill a foreground run).
                if (terminateOnTimeout && worker) {
                    worker.terminate();
                    worker = null;
                }
                reject(new Error(terminateOnTimeout ? "timed out" : "timed out while loading the playground"));
            }, timeoutMs);
            pending.set(id, { resolve: resolve, reject: reject, timer: timer });
            try {
                ensureWorker().postMessage(message);
            } catch (error) {
                pending.delete(id);
                clearTimeout(timer);
                reject(error);
            }
        });
    }

    // Starts loading the compiler in the background. Safe to call repeatedly.
    // Does nothing on unsupported browsers (run reports it instead).
    function warmUp() {
        if (!isSupported()) return;
        postMessage({ action: "ping" }, WARMUP_TIMEOUT_MS).catch(function () {
            // Warmup failures surface on the first actual run instead.
        });
    }

    // Compiles and runs cx source code, resolving to {stdout, stderr}.
    // Never rejects: internal failures are reported via stderr.
    function run(code) {
        if (!isSupported()) {
            return Promise.resolve({ stdout: "", stderr: unsupportedMessage() });
        }
        return postMessage({ action: "run", code: code }, RUN_TIMEOUT_MS, true).then(function (message) {
            if (message.ready !== undefined && !message.ready) {
                return { stdout: "", stderr: "error: failed to load the playground (" + (message.error || "unknown error") + ")\n" };
            }
            return { stdout: message.stdout || "", stderr: message.stderr || "" };
        }, function (error) {
            var message = "" + (error && error.message ? error.message : error);
            if (message === "timed out") {
                return {
                    stdout: "",
                    stderr: "error: timed out (the program may contain an infinite loop)\n",
                };
            }
            return { stdout: "", stderr: "error: the playground failed to run (" + message + ")\n" };
        });
    }

    // Compiles cx source without running it, resolving to {stdout, stderr}
    // with the compiler diagnostics. Never rejects; internal failures
    // resolve to empty diagnostics so background checks stay silent (the
    // Run button reports playground failures instead).
    function check(code) {
        if (!isSupported()) {
            return Promise.resolve({ stdout: "", stderr: "" });
        }
        return postMessage({ action: "check", code: code }, CHECK_TIMEOUT_MS).then(function (message) {
            if (message.ready !== undefined && !message.ready) {
                return { stdout: "", stderr: "" };
            }
            return { stdout: message.stdout || "", stderr: message.stderr || "" };
        }, function () {
            return { stdout: "", stderr: "" };
        });
    }

    global.CxPlayground = {
        warmUp: warmUp,
        run: run,
        check: check,
        isSupported: isSupported,
    };
})(typeof globalThis !== "undefined" ? globalThis : this);
