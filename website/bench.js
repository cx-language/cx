// Benchmark history graphs. Data is bench-data.json: an array of records
// {sha, timestamp, metrics} in commit order, attached by the website deploy.
(function () {
    "use strict";

    var COMMIT_URL = "https://github.com/cx-language/cx/commit/";
    var COMPARE_URL = "https://github.com/cx-language/cx/compare/";
    var COLORS = ["#3b82f6", "#ef4444", "#22c55e", "#f59e0b", "#a855f7", "#06b6d4", "#ec4899"];
    var PAD = { left: 64, right: 16, top: 12, bottom: 26 };
    var HEIGHT = 260;

    function cssVar(name) {
        return getComputedStyle(document.documentElement).getPropertyValue(name).trim();
    }

    function fmtSeconds(v) {
        if (v >= 100) return v.toFixed(0) + "s";
        if (v >= 10) return v.toFixed(1) + "s";
        return v.toFixed(2) + "s";
    }

    function fmtBytes(v) {
        if (v >= 1048576) return (v / 1048576).toFixed(1) + " MB";
        if (v >= 1024) return (v / 1024).toFixed(0) + " KB";
        return v + " B";
    }

    function fmtInt(v) {
        return Math.round(v).toLocaleString("en-US");
    }

    function fmtDate(iso) {
        var d = new Date(iso);
        return (d.getMonth() + 1) + "-" + d.getDate();
    }

    function get(metrics, path) {
        var v = metrics;
        for (var i = 0; i < path.length; i++) {
            if (v === null || v === undefined) return null;
            v = v[path[i]];
        }
        return typeof v === "number" ? v : null;
    }

    function drawChart(legendId, tip, records, series, fmt) {
        // The canvas is created here because pandoc strips canvas elements
        // from the page source on its way through the website build.
        var legendEl = document.getElementById(legendId);
        var canvas = document.createElement("canvas");
        legendEl.parentNode.appendChild(canvas);
        canvas.style.height = HEIGHT + "px";
        var ctx = canvas.getContext("2d");

        var n = records.length;
        var values = series.map(function (s) {
            return records.map(function (r) {
                return get(r.metrics, s.path);
            });
        });
        var flat = values.flat().filter(function (v) {
            return v !== null;
        });
        var fg = cssVar("--text-color") || "#000";
        function sizeCanvas() {
            var dpr = window.devicePixelRatio || 1;
            canvas.width = Math.round(canvas.clientWidth * dpr);
            canvas.height = Math.round(HEIGHT * dpr);
            ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
        }
        if (!flat.length) {
            sizeCanvas();
            ctx.font = "11px system-ui, sans-serif";
            ctx.fillStyle = fg;
            ctx.fillText("no data", PAD.left, PAD.top + 14);
            legend();
            return function () {};
        }
        var min = Math.min.apply(null, flat);
        var max = Math.max.apply(null, flat);
        if (min === max) {
            min -= 1;
            max += 1;
        }
        var span = max - min;
        min -= span * 0.1;
        max += span * 0.1;
        // All metrics are non-negative; keep the axis honest near zero.
        if (min < 0) min = 0;

        var selected = -1;
        // Sizing lives inside draw so resize redraws at the new geometry
        // instead of stretching the old backing store.
        function draw() {
            sizeCanvas();
            var width = canvas.clientWidth;
            var plotW = width - PAD.left - PAD.right;
            var plotH = HEIGHT - PAD.top - PAD.bottom;
            function x(i) {
                return n === 1 ? PAD.left + plotW / 2 : PAD.left + (i * plotW) / (n - 1);
            }
            function y(v) {
                return PAD.top + plotH - ((v - min) / (max - min)) * plotH;
            }
            ctx.font = "11px system-ui, sans-serif";
            ctx.clearRect(0, 0, width, HEIGHT);
            ctx.strokeStyle = "rgba(128, 128, 128, 0.35)";
            ctx.fillStyle = fg;
            ctx.lineWidth = 1;
            ctx.textAlign = "right";
            for (var t = 0; t <= 4; t++) {
                var gv = min + ((max - min) * t) / 4;
                var gy = Math.round(y(gv)) + 0.5;
                ctx.beginPath();
                ctx.moveTo(PAD.left, gy);
                ctx.lineTo(width - PAD.right, gy);
                ctx.stroke();
                ctx.fillText(fmt(gv), PAD.left - 6, gy + 4);
            }
            ctx.textAlign = "center";
            var ticks = Math.min(n, 6);
            for (var k = 0; k < ticks; k++) {
                var idx = Math.round((k * (n - 1)) / (ticks - 1 || 1));
                ctx.fillText(fmtDate(records[idx].timestamp), x(idx), HEIGHT - 8);
            }
            series.forEach(function (s, si) {
                ctx.strokeStyle = COLORS[si % COLORS.length];
                ctx.lineWidth = 2;
                ctx.beginPath();
                var pen = false;
                values[si].forEach(function (v, i) {
                    if (v === null) {
                        pen = false;
                        return;
                    }
                    if (!pen) {
                        ctx.moveTo(x(i), y(v));
                        pen = true;
                    } else {
                        ctx.lineTo(x(i), y(v));
                    }
                });
                ctx.stroke();
                ctx.fillStyle = COLORS[si % COLORS.length];
                values[si].forEach(function (v, i) {
                    if (v === null) return;
                    ctx.beginPath();
                    ctx.arc(x(i), y(v), i === selected ? 4.5 : 2.5, 0, 7);
                    ctx.fill();
                });
            });
            if (selected >= 0) {
                ctx.strokeStyle = "rgba(128, 128, 128, 0.6)";
                ctx.lineWidth = 1;
                var sx = Math.round(x(selected)) + 0.5;
                ctx.beginPath();
                ctx.moveTo(sx, PAD.top);
                ctx.lineTo(sx, PAD.top + plotH);
                ctx.stroke();
            }
        }

        function legend() {
            series.forEach(function (s, si) {
                var item = document.createElement("span");
                var swatch = document.createElement("i");
                swatch.style.background = COLORS[si % COLORS.length];
                item.appendChild(swatch);
                item.appendChild(document.createTextNode(s.label));
                legendEl.appendChild(item);
            });
        }

        canvas.addEventListener("mousemove", function (ev) {
            var rect = canvas.getBoundingClientRect();
            var plotW = canvas.clientWidth - PAD.left - PAD.right;
            var step = n === 1 ? 1 : plotW / (n - 1);
            var i = Math.round((ev.clientX - rect.left - PAD.left) / step);
            selected = Math.max(0, Math.min(n - 1, i));
            draw();
            var r = records[selected];
            var html = "<b>" + fmtDate(r.timestamp) + "</b> " + '<span class="tip-sha">' + r.sha.slice(0, 7) + "</span>";
            series.forEach(function (s, si) {
                var v = values[si][selected];
                if (v !== null) html += "<br>" + s.label + ": " + fmt(v);
            });
            tip.innerHTML = html;
            tip.hidden = false;
            var tw = tip.offsetWidth;
            tip.style.left = Math.min(ev.clientX + 14, window.innerWidth - tw - 8) + "px";
            tip.style.top = ev.clientY + 12 + "px";
        });
        canvas.addEventListener("mouseleave", function () {
            selected = -1;
            tip.hidden = true;
            draw();
        });
        canvas.addEventListener("click", function () {
            if (selected < 0) return;
            // Consecutive data points can be many commits apart, so link the
            // range, not the single commit. The first point has no previous
            // record to compare against.
            if (selected === 0) {
                window.open(COMMIT_URL + records[0].sha, "_blank");
            } else {
                window.open(COMPARE_URL + records[selected - 1].sha + "..." + records[selected].sha, "_blank");
            }
        });

        legend();
        draw();
        return draw;
    }

    function subSeries(records, group) {
        // Union keys across all records in first-seen order: programs added
        // to the corpus later have no entries in early records. drawChart
        // already renders missing points as gaps in the line.
        var keys = [];
        records.forEach(function (r) {
            Object.keys((r.metrics && r.metrics[group]) || {}).forEach(function (key) {
                if (keys.indexOf(key) === -1) keys.push(key);
            });
        });
        return keys.map(function (key) {
            return { path: [group, key], label: key };
        });
    }

    function showEmpty() {
        document.getElementById("bench-charts").innerHTML =
            "<p>No benchmark data yet. Records appear here once CI has benched " +
            "commits on the main branch.</p>";
    }

    var LANGS = {
        cx: { label: "cx", color: null },
        c: { label: "C", color: "#555555" },
        cxx: { label: "C++", color: "#f34b7d" },
        rust: { label: "Rust", color: "#dea584" },
        go: { label: "Go", color: "#00ADD8" },
        odin: { label: "Odin", color: "#60AFFE" },
        zig: { label: "Zig", color: "#ec915c" },
        swift: { label: "Swift", color: "#F05138" },
    };
    var MODE_LABEL = { release: "optimized", debug: "unoptimized debug" };

    function fmtLangSeconds(seconds) {
        if (seconds < 1) return Math.round(seconds * 1000) + " ms";
        return seconds.toFixed(3) + " s";
    }

    function escapeHtml(s) {
        return String(s)
            .replace(/&/g, "&amp;")
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/"/g, "&quot;");
    }

    function langLabel(lang) {
        return (LANGS[lang] && LANGS[lang].label) || lang;
    }

    function langColor(lang) {
        if (lang === "cx") return cssVar("--text-color") || "#000";
        return (LANGS[lang] && LANGS[lang].color) || "#888";
    }

    function chartSVG(medians) {
        // Horizontal bars, fastest first. Mirrors chart_svg in scripts/bench-langs.py.
        var entries = Object.keys(medians).map(function (lang) {
            return [lang, medians[lang]];
        });
        entries.sort(function (a, b) {
            return a[1] - b[1];
        });
        var fastest = entries[0][1];
        var max = entries.reduce(function (m, e) {
            return Math.max(m, e[1]);
        }, 0);
        var rowH = 30, labelW = 64, valueW = 150, width = 760;
        var barW = width - labelW - valueW;
        var rows = entries.map(function (entry, i) {
            var lang = entry[0], seconds = entry[1];
            var y = i * rowH;
            var length = Math.max(2, (seconds / max) * barW);
            return (
                '<text x="0" y="' + (y + 20) + '">' + escapeHtml(langLabel(lang)) + "</text>" +
                '<rect x="' + labelW + '" y="' + (y + 6) + '" width="' + length.toFixed(1) +
                '" height="18" style="fill:' + langColor(lang) + '"/>' +
                '<text x="' + (labelW + length + 8).toFixed(1) + '" y="' + (y + 20) + '" class="value">' +
                fmtLangSeconds(seconds) + " (" + (seconds / fastest).toFixed(2) + "x)</text>"
            );
        });
        var height = entries.length * rowH + 6;
        return '<svg viewBox="0 0 ' + width + " " + height + '" width="100%" role="img">' + rows.join("") + "</svg>";
    }

    function renderLangs(record) {
        var metaEl = document.getElementById("langs-meta");
        var chartsEl = document.getElementById("langs-charts");
        var buildsEl = document.getElementById("langs-builds");
        var parts = [];
        var metrics = record.metrics || [];
        if (metrics.indexOf("run") !== -1) parts.push("median of " + record.runs + " runs");
        if (metrics.indexOf("compile") !== -1) parts.push("median of " + record.compile_runs + " debug compiles");
        var tools = Object.keys(record.tools || {})
            .map(function (lang) {
                return escapeHtml(lang) + ": " + escapeHtml(record.tools[lang]);
            })
            .join(" · ");
        var meta =
            escapeHtml(String(record.timestamp).slice(0, 10)) +
            " · " + escapeHtml(record.platform || "") +
            " · " + escapeHtml(parts.join(" · "));
        if (tools) meta += "<br>" + tools;
        if (record.cx_sha) meta += '<br>cx at <a href="' + COMMIT_URL + escapeHtml(record.cx_sha) + '">' + escapeHtml(String(record.cx_sha).slice(0, 7)) + "</a>";
        metaEl.innerHTML = meta;
        var html = "";
        var allPrograms = record.programs || {};
        var modes = record.mode_order || [];
        record.program_order.forEach(function (program) {
            var byMode = allPrograms[program] || {};
            var note =
                program === "mandelbrot"
                    ? ' <span class="langs-note">checksums differ by design here (float-to-int conversion is ' +
                      "platform-defined); times remain comparable.</span>"
                    : "";
            var omitted = (record.omission_notes && record.omission_notes[program]) || "";
            var omittedHtml = omitted ? '<p class="langs-note">' + escapeHtml(omitted) + "</p>" : "";
            var charts = "";
            modes.forEach(function (mode) {
                var entries = byMode[mode] || {};
                var runMedians = {};
                Object.keys(entries).forEach(function (lang) {
                    if (entries[lang] && typeof entries[lang].median_s === "number") runMedians[lang] = entries[lang].median_s;
                });
                if (Object.keys(runMedians).length)
                    charts += "<h4>" + escapeHtml(MODE_LABEL[mode] || mode) + " run</h4>\n" + chartSVG(runMedians);
                var compileMedians = {};
                Object.keys(entries).forEach(function (lang) {
                    if (entries[lang] && entries[lang].compile && typeof entries[lang].compile.median_s === "number")
                        compileMedians[lang] = entries[lang].compile.median_s;
                });
                if (Object.keys(compileMedians).length)
                    charts += "<h4>" + escapeHtml(MODE_LABEL[mode] || mode) + " compile</h4>\n" + chartSVG(compileMedians);
            });
            if (!charts) {
                html += "<h3>" + escapeHtml(program) + "</h3>" + omittedHtml + "<p>no successful measurements</p>";
            } else {
                html += '<div class="langs-chart"><h3>' + escapeHtml(program) + note + "</h3>" + omittedHtml + charts + "</div>";
            }
        });
        chartsEl.innerHTML = html;
        var builds = "";
        modes.forEach(function (mode) {
            var items = "";
            Object.keys((record.builds && record.builds[mode]) || {}).forEach(function (lang) {
                items += "<li>" + escapeHtml(langLabel(lang)) + ": <code>" + escapeHtml(record.builds[mode][lang]) + "</code></li>";
            });
            builds += "<h4>" + escapeHtml(MODE_LABEL[mode] || mode) + "</h4><ul>" + items + "</ul>";
        });
        // Built here because pandoc strips details elements from the page source.
        buildsEl.innerHTML = "<details><summary>Build configurations</summary>" + builds + "</details>";
    }

    function showLangsEmpty() {
        document.getElementById("langs-charts").innerHTML =
            "<p>No language comparison data yet. It appears here once CI has benched " +
            "the main branch.</p>";
    }

    fetch("bench-data.json")
        .then(function (resp) {
            if (!resp.ok) throw new Error("no data");
            return resp.json();
        })
        .then(function (records) {
            if (!records.length) {
                showEmpty();
                return;
            }
            var tip = document.getElementById("bench-tip");
            var redraws = [
                drawChart("legend-build", tip, records, [
                    { path: ["cxx_build_s"], label: "C++ build" },
                    { path: ["check_s"], label: "test suite" },
                ], fmtSeconds),
                drawChart("legend-compile", tip, records,
                    subSeries(records, "compile_s"), fmtSeconds),
                drawChart("legend-run", tip, records,
                    subSeries(records, "run_s"), fmtSeconds),
                drawChart("legend-cxsize", tip, records,
                    [{ path: ["cx_bytes"], label: "cx" }], fmtBytes),
                drawChart("legend-benchsize", tip, records,
                    subSeries(records, "bench_bytes"), fmtBytes),
                drawChart("legend-sloc", tip, records, [
                    { path: ["sloc", "total"], label: "total" },
                    { path: ["sloc", "compiler"], label: "compiler" },
                    { path: ["sloc", "stdlib"], label: "stdlib" },
                    { path: ["sloc", "vendor"], label: "vendor" },
                    { path: ["sloc", "docs"], label: "docs" },
                    { path: ["sloc", "examples"], label: "examples" },
                    { path: ["sloc", "tests"], label: "tests" },
                ], fmtInt),
            ];
            window.addEventListener("resize", function () {
                redraws.forEach(function (draw) {
                    draw();
                });
            });
        })
        .catch(showEmpty);

    fetch("langs-data.json")
        .then(function (resp) {
            if (!resp.ok) throw new Error("no data");
            return resp.json();
        })
        .then(function (record) {
            if (!record || !record.program_order || !record.program_order.length) {
                showLangsEmpty();
                return;
            }
            renderLangs(record);
        })
        .catch(showLangsEmpty);
})();
