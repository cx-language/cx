document.addEventListener("DOMContentLoaded", function() {
    const codeBlocks = document.querySelectorAll("pre.sourceCode:not(.sh)");
    for (const block of codeBlocks) {
        initializeCodeEditor(block);
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
    runButton.innerText = "▶\uFE0E";
    runButton.className = "run";
    editorWrapper.appendChild(runButton);

    const output = document.createElement("div");
    output.className = "output";
    editorWrapper.appendChild(output);

    const stdout = document.createElement("div");
    stdout.className = "stdout";
    output.appendChild(stdout);

    const stderr = document.createElement("div");
    stderr.className = "stderr";
    output.appendChild(stderr);

    block.parentNode.replaceChild(editorWrapper, block);

    var widgets = [];

    function highlightError() {
        var regex = /^main\.cx:(\d+):(\d+): (.*)(?:\n.*\n([ \t]*)\^)?/gm;
        var match;
        while ((match = regex.exec(output.innerText))) {
            var [, line, column, message, indent] = match;
            console.log(match, indent);
            var node = document.createElement("div");
            node.appendChild(document.createTextNode(indent + "^ " + message));
            node.className = "error";
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
        removeErrors();
        output.style.display = "block";
        output.scrollIntoView({ behavior: "smooth", block: "nearest" });

        stdout.innerText = "Running...";
        stderr.innerText = "";
        var outputUpdater = setInterval(function() {
            stdout.innerText += ".";
        }, 1000);

        var xhr = new XMLHttpRequest();
        xhr.open("POST", "https://cx-online-compiler.herokuapp.com/run", true);
        xhr.onerror = function(error) {
            clearInterval(outputUpdater);
            stdout.innerText = "";
            stderr.innerText = "Error: " + error.message;
        };
        xhr.onload = function() {
            if (xhr.readyState === 4) {
                clearInterval(outputUpdater);
                if (xhr.status === 200) {
                    var response = JSON.parse(xhr.response);
                    stdout.innerText = response.stdout || "";
                    stderr.innerText = response.stderr || "";
                    highlightError();
                } else {
                    stdout.innerText = "";
                    stderr.innerText = "Error: " + xhr.statusText;
                }
                output.scrollIntoView({ behavior: "smooth", block: "nearest" });
            }
        };
        xhr.setRequestHeader("Content-Type", "application/json");
        xhr.send(JSON.stringify({ code: editor.getValue() }));
    };
}
