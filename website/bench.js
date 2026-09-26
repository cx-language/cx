// Benchmark history graphs. Data is bench-data.json: an array of records
// {sha, timestamp, metrics} in commit order, attached by the website deploy.
(function () {
    "use strict";

    var COMMIT_URL = "https://github.com/cx-language/cx/commit/";
    var COLORS = ["#3b82f6", "#ef4444", "#22c55e", "#f59e0b", "#a855f7", "#06b6d4"];
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
            if (selected >= 0) window.open(COMMIT_URL + records[selected].sha, "_blank");
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
            ];
            window.addEventListener("resize", function () {
                redraws.forEach(function (draw) {
                    draw();
                });
            });
        })
        .catch(showEmpty);
})();
