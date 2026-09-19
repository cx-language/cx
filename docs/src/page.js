// Turns documentation code blocks into runnable playground editors.
//
// `createEditor` builds the code editor hosted in `wrapper` showing
// `initialText`, and returns its {getValue, setValue, setDiagnostics}
// interface. It is injected so tests can substitute a fake.

import { parseDiagnostics } from "./diagnostics.js";

export function initializeAllCodeEditors(createEditor) {
    const codeBlocks = document.querySelectorAll("pre.sourceCode:not(.sh):not(.noRun)");
    for (const block of codeBlocks) {
        initializeCodeEditor(block, createEditor);
    }
    if (typeof CxPlayground !== "undefined") {
        CxPlayground.warmUp();
    }
}

export function initializeCodeEditor(block, createEditor) {
    const editorWrapper = document.createElement("div");
    editorWrapper.className = "editor";

    const editor = createEditor(editorWrapper, block.innerText);

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
    // examples. Its <select> is baked into index.html at website build time;
    // the example sources come from playground-examples.js.
    var showcase = editorWrapper.closest("div.showcase");
    if (showcase) {
        var selector = showcase.querySelector("#example-selector");
        if (selector) {
            if (typeof CxExamples === "undefined") {
                selector.style.display = "none";
            } else {
                // Chromium keeps showing the focus ring on selects after mouse
                // selection, so drop focus then. Keyboard selection keeps focus
                // so arrow keys keep navigating the options.
                var selectedWithPointer = false;
                selector.addEventListener("pointerdown", function() {
                    selectedWithPointer = true;
                });
                selector.addEventListener("keydown", function() {
                    selectedWithPointer = false;
                });
                selector.onchange = function() {
                    if (selectedWithPointer) {
                        selectedWithPointer = false;
                        selector.blur();
                    }
                    var example = CxExamples[Number(selector.value)];
                    if (!example) return;
                    editor.setValue(example.code);
                    editor.setDiagnostics([]);
                    output.style.display = "none";
                    stdout.innerText = "";
                    stderr.innerText = "";
                    runButton.click();
                };
            }
        }
    }

    runButton.onclick = function() {
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

        editor.setDiagnostics([]);
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
            editor.setDiagnostics(parseDiagnostics(output.innerText));
            output.scrollIntoView({ behavior: "smooth", block: "nearest" });
        });
    };
}
