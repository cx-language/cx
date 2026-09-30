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

function initializeCodeEditor(block) {
    const editorWrapper = document.createElement("div");
    editorWrapper.className = "editor";

    const editor = CodeMirror(editorWrapper, {
        mode: "cx",
        theme: "cx",
        indentUnit: 4,
        value: block.innerText,
        viewportMargin: Infinity
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
    editor.on("change", function() {
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
