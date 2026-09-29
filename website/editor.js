document.addEventListener("DOMContentLoaded", function() {
    // Static snippets (front-page cards): highlight only, no Run button.
    document.querySelectorAll("pre.snippet.cx").forEach(function(pre) {
        CodeMirror(function(node) {
            node.classList.add("snippet");
            pre.replaceWith(node);
        }, {
            mode: "cx",
            theme: "cx",
            value: pre.innerText,
            readOnly: "nocursor",
            viewportMargin: Infinity
        });
    });
    const codeBlocks = document.querySelectorAll("pre.sourceCode:not(.sh):not(.noRun)");
    for (const block of codeBlocks) {
        initializeCodeEditor(block);
    }
    if (typeof CxPlayground !== "undefined") {
        CxPlayground.warmUp();
    }
});

// CodeMirror autocompletion backed by the playground worker (see
// playground.js): Ctrl-Space completes at the cursor, typing "." completes
// members automatically. The analyzer returns all in-scope items; the
// prefix filtering below is client-side, like other LSP clients.
var lastCompletion = null;
var completionWaiters = [];
var completionTimer = null;
var completionInflight = false;
var COMPLETION_DEBOUNCE_MS = 250;

function isCompletionPrefixExtension(cached, code, line, ch) {
    // True when the only change since the cached query is identifier
    // characters appended at the cursor on the same line: the completion
    // context is unchanged, so the cached items still apply and typing
    // doesn't recompile on every keystroke. (Corner: typing the last
    // letter of a keyword keeps the pre-keyword items until the next
    // query. Harmless: the typed text still matches the list.)
    // The cache is keyed on the full code, so sharing it across the
    // page's editors is sound: identical code and cursor means
    // identical completions.
    if (line !== cached.line || ch < cached.ch) return false;
    var oldLines = cached.code.split("\n");
    var newLines = code.split("\n");
    if (oldLines.length !== newLines.length) return false;
    for (var i = 0; i < newLines.length; i++) {
        if (i !== line && oldLines[i] !== newLines[i]) return false;
    }
    var oldLine = oldLines[line];
    var newLine = newLines[line];
    if (newLine.length - oldLine.length !== ch - cached.ch) return false;
    if (newLine.slice(0, cached.ch) !== oldLine.slice(0, cached.ch)) return false;
    if (newLine.slice(ch) !== oldLine.slice(cached.ch)) return false;
    return /^\w*$/.test(newLine.slice(cached.ch, ch));
}

function fireCompletion() {
    completionTimer = null;
    if (completionInflight || completionWaiters.length === 0) return;
    completionInflight = true;
    // Every waiter resolves with the latest query's items; waiters from
    // older cursors filter by their own prefix and show-hint drops the
    // stale ones by tick.
    var latest = completionWaiters[completionWaiters.length - 1];
    var waiters = completionWaiters;
    completionWaiters = [];
    CxPlayground.complete(latest.code, latest.line, latest.ch).then(function (response) {
        var items = response.items || [];
        // Failures resolve to no items upstream; never cache those, or
        // typing on would keep reusing the empty list.
        if (!response.failed) {
            lastCompletion = { code: latest.code, line: latest.line, ch: latest.ch, items: items };
        }
        completionInflight = false;
        waiters.forEach(function (waiter) {
            waiter.resolve(items);
        });
        // Queries queued behind the in-flight one fire now.
        if (completionWaiters.length !== 0 && completionTimer === null) {
            completionTimer = setTimeout(fireCompletion, COMPLETION_DEBOUNCE_MS);
        }
    });
}

function fetchCompletions(code, line, ch) {
    if (lastCompletion && isCompletionPrefixExtension(lastCompletion, code, line, ch)) {
        return Promise.resolve(lastCompletion.items);
    }
    // Debounced and at most one in flight: each query instantiates a
    // fresh compiler, so fast typing must not pile up compiles.
    return new Promise(function (resolve) {
        completionWaiters.push({ resolve: resolve, code: code, line: line, ch: ch });
        if (completionTimer === null) {
            completionTimer = setTimeout(fireCompletion, COMPLETION_DEBOUNCE_MS);
        }
    });
}

function renderCompletion(elt, data, completion) {
    elt.appendChild(document.createTextNode(completion.text));
    if (completion.detail) {
        var detail = document.createElement("span");
        detail.className = "CodeMirror-hint-detail";
        detail.appendChild(document.createTextNode("  " + completion.detail));
        elt.appendChild(detail);
    }
}

function cxHint(cm) {
    var cursor = cm.getCursor();
    var lineText = cm.getLine(cursor.line);
    var start = cursor.ch;
    while (start > 0 && /\w/.test(lineText.charAt(start - 1))) start--;
    var end = cursor.ch;
    while (end < lineText.length && /\w/.test(lineText.charAt(end))) end++;
    var prefix = lineText.slice(start, cursor.ch);
    var code = cm.getValue();
    var from = CodeMirror.Pos(cursor.line, start);
    var to = CodeMirror.Pos(cursor.line, end);

    var fresh = { list: [], from: from, to: to };
    if (typeof CxPlayground === "undefined" || !CxPlayground.complete) {
        return Promise.resolve(fresh);
    }
    return fetchCompletions(code, cursor.line, cursor.ch).then(function (items) {
        var list = items.filter(function (item) {
            return typeof item.label === "string" && item.label.lastIndexOf(prefix, 0) === 0;
        }).map(function (item) {
            var completion = { text: item.label, displayText: item.label, render: renderCompletion };
            if (item.detail) completion.detail = item.detail;
            return completion;
        });
        return { list: list, from: from, to: to };
    });
}

if (typeof CodeMirror !== "undefined" && CodeMirror.registerHelper) {
    CodeMirror.registerHelper("hint", "cx", cxHint);
}

function initializeCodeEditor(block) {
    const editorWrapper = document.createElement("div");
    editorWrapper.className = "editor";

    const editor = CodeMirror(editorWrapper, {
        mode: "cx",
        theme: "cx",
        indentUnit: 4,
        value: block.innerText,
        viewportMargin: Infinity,
        extraKeys: { "Ctrl-Space": "autocomplete" },
        hintOptions: { completeSingle: false }
    });
    setTimeout(function() {
        editor.refresh();
    }, 1);

    const runButton = document.createElement("button");
    runButton.setAttribute("aria-label", "Run");
    runButton.className = "run";
    block.parentNode.appendChild(runButton);

    const output = document.createElement("div");
    output.className = "output";
    block.parentNode.appendChild(output);

    const stdout = document.createElement("div");
    stdout.className = "stdout";
    output.appendChild(stdout);

    const stderr = document.createElement("div");
    stderr.className = "stderr";
    output.appendChild(stderr);

    block.parentNode.replaceChild(editorWrapper, block);

    // The front-page showcase lets the reader switch between runnable
    // examples. Its tab bar is baked into index.html at website build time;
    // the example sources come from playground-examples.js.
    var showcase = editorWrapper.closest("div.showcase");
    if (showcase) {
        var tabs = showcase.querySelector(".example-tabs");
        if (tabs) {
            if (typeof CxExamples === "undefined") {
                tabs.style.display = "none";
            } else {
                var buttons = tabs.querySelectorAll("button");
                tabs.addEventListener("click", function(event) {
                    var button = event.target.closest("button");
                    if (!button) return;
                    buttons.forEach(function(other) {
                        other.setAttribute("aria-pressed", other === button ? "true" : "false");
                    });
                    var example = CxExamples[Number(button.getAttribute("data-tab"))];
                    if (!example) return;
                    editor.setValue(example.code);
                    removeErrors();
                    output.style.display = "none";
                    stdout.innerText = "";
                    stderr.innerText = "";
                    runButton.click();
                });
            }
        }
    }

    var widgets = [];
    var runCount = 0;

    function highlightError(diagnostics) {
        // The compiler underlines ranges with '~' and points with '^';
        // diagnostics without source context have neither.
        var regex = /^main\.cx:(\d+):(\d+): (.*)(?:\n.*\n([ \t]*)[~^])?/gm;
        var match;
        while ((match = regex.exec(diagnostics))) {
            var [, line, column, message, indent] = match;
            var node = document.createElement("div");
            // The strip styling already signals severity; drop the prefix.
            var compact = message.replace(/^(error|warning): /, "");
            node.appendChild(document.createTextNode((indent || "") + "^ " + compact));
            node.classList.add("diagnostic", message.startsWith("warning") ? "warning" : "error");
            widgets.push(editor.addLineWidget(line - 1, node, true));
        }
    }

    function removeErrors() {
        for (var i = 0; i < widgets.length; ++i) {
            widgets[i].clear();
        }
        widgets.length = 0;
    }

    runButton.onclick = function() {
        runCount++;
        if (typeof CxPlayground === "undefined") {
            output.style.display = "block";
            stdout.innerText = "";
            stderr.innerText = "Error: the playground is unavailable (JavaScript files missing from this page).";
            return;
        }

        if (!CxPlayground.isSupported()) {
            output.style.display = "block";
            stdout.innerText = "";
            stderr.innerText = "Error: the online playground needs WebAssembly, which your browser doesn't support. " +
                "Please try a recent version of Chrome, Firefox, Safari, or Edge.";
            return;
        }

        removeErrors();
        output.style.display = "block";
        output.scrollIntoView({ behavior: "smooth", block: "nearest" });

        runButton.disabled = true;
        stdout.innerText = "Running...";
        stderr.innerText = "";
        var outputUpdater = setInterval(function() {
            stdout.innerText += ".";
        }, 1000);

        CxPlayground.run(editor.getValue()).then(function(response) {
            clearInterval(outputUpdater);
            runButton.disabled = false;
            stdout.innerText = response.stdout || "";
            stderr.innerText = response.stderr || "";
            removeErrors();
            highlightError(output.innerText);
            output.scrollIntoView({ behavior: "smooth", block: "nearest" });
        });
    };

    // Live diagnostics: recompile (without running) after the user stops
    // typing, so errors show without pressing the play button. Only the
    // latest check paints: an older check resolving late, or any check
    // superseded by a Run, is discarded.
    var checkTimer = null;
    var checkSequence = 0;
    editor.on("change", function(cm, change) {
        // Typing "." completes members automatically; anything else the
        // analyzer can't complete for resolves to no popup.
        if (change && change.origin === "+input" && change.text.join("") === "." && change.removed.join("") === "" &&
            typeof CxPlayground !== "undefined" && CxPlayground.complete && editor.showHint) {
            editor.showHint();
        }
        if (typeof CxPlayground === "undefined" || !CxPlayground.check) return;
        clearTimeout(checkTimer);
        var runStamp = runCount;
        checkTimer = setTimeout(function() {
            if (runStamp !== runCount) return;
            var sequence = ++checkSequence;
            CxPlayground.check(editor.getValue()).then(function(response) {
                if (sequence !== checkSequence || runStamp !== runCount) return;
                removeErrors();
                highlightError(response.stderr || "");
            });
        }, 750);
    });
}
