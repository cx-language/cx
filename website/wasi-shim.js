// Minimal WASI (wasi_snapshot_preview1) implementation for the cx playground.
//
// Runs both the in-browser C compiler (cc.wasm, built from xcc's wcc) and the
// user programs it produces. It provides an in-memory file system and just
// enough system calls for those two use cases: regular files and directories,
// stdin (empty), and captured stdout/stderr.
//
// This file intentionally has no dependencies and works both in browsers
// (loaded via <script> or importScripts) and in Node.js (for tests), where it
// is exposed as CxWasi / module.exports respectively.

(function (global) {
    "use strict";

    // WASI errno values (wasi_snapshot_preview1).
    var ERRNO_SUCCESS = 0;
    var ERRNO_BADF = 8;
    var ERRNO_EXIST = 20;
    var ERRNO_INVAL = 28;
    var ERRNO_ISDIR = 31;
    var ERRNO_NOENT = 44;
    var ERRNO_NOTDIR = 54;
    var ERRNO_NOTEMPTY = 55;

    var FILETYPE_DIRECTORY = 3;
    var FILETYPE_REGULAR_FILE = 4;

    var OFLAGS_CREAT = 1;
    var OFLAGS_DIRECTORY = 2;
    var OFLAGS_EXCL = 4;
    var OFLAGS_TRUNC = 8;

    var WHENCE_SET = 0;
    var WHENCE_CUR = 1;
    var WHENCE_END = 2;

    // Signal normal process termination through proc_exit.
    function ExitError(code) {
        this.code = code;
    }

    function normalizePath(path) {
        // Resolve relative to the process working directory, which is always
        // "/" in the playground (there is a single preopen).
        if (path[0] !== "/") {
            path = "/" + path;
        }
        var parts = path.split("/");
        var out = [];
        for (var i = 0; i < parts.length; i++) {
            var part = parts[i];
            if (part === "" || part === ".") continue;
            if (part === "..") {
                if (out.length > 0) out.pop();
            } else {
                out.push(part);
            }
        }
        return "/" + out.join("/");
    }

    function dirname(path) {
        if (path === "/") return "/";
        var index = path.lastIndexOf("/");
        if (index <= 0) return "/";
        return path.slice(0, index);
    }

    // In-memory file system: path -> {type, data, mtime}.
    function FileSystem() {
        this.entries = new Map();
        this.entries.set("/", { type: "dir", children: new Set() });
    }

    FileSystem.prototype._addToParent = function (path) {
        var parent = this.entries.get(dirname(path));
        if (parent && parent.type === "dir") {
            parent.children.add(path);
        }
    };

    FileSystem.prototype.mkdir = function (path) {
        path = normalizePath(path);
        var existing = this.entries.get(path);
        if (existing) {
            if (existing.type !== "dir") return ERRNO_NOTDIR;
            return ERRNO_SUCCESS; // Like mkdir -p.
        }
        var parent = this.entries.get(dirname(path));
        if (!parent) return ERRNO_NOENT;
        if (parent.type !== "dir") return ERRNO_NOTDIR;
        this.entries.set(path, { type: "dir", children: new Set() });
        this._addToParent(path);
        return ERRNO_SUCCESS;
    };

    FileSystem.prototype.mkdirs = function (path) {
        path = normalizePath(path);
        var parts = path.split("/").filter(function (part) { return part !== ""; });
        var current = "";
        for (var i = 0; i < parts.length; i++) {
            current += "/" + parts[i];
            var result = this.mkdir(current);
            if (result !== ERRNO_SUCCESS) return result;
        }
        return ERRNO_SUCCESS;
    };

    FileSystem.prototype.writeFile = function (path, data) {
        path = normalizePath(path);
        if (typeof data === "string") {
            data = new TextEncoder().encode(data);
        }
        var existing = this.entries.get(path);
        if (existing && existing.type === "dir") return ERRNO_ISDIR;
        var parent = this.entries.get(dirname(path));
        if (!parent) return ERRNO_NOENT;
        if (parent.type !== "dir") return ERRNO_NOTDIR;
        this.entries.set(path, { type: "file", data: new Uint8Array(data) });
        this._addToParent(path);
        return ERRNO_SUCCESS;
    };

    FileSystem.prototype.readFile = function (path) {
        path = normalizePath(path);
        var entry = this.entries.get(path);
        if (!entry) return null;
        if (entry.type === "dir") return null;
        return entry.data;
    };

    FileSystem.prototype.unlink = function (path) {
        path = normalizePath(path);
        var entry = this.entries.get(path);
        if (!entry) return ERRNO_NOENT;
        if (entry.type === "dir") {
            if (entry.children.size > 0) return ERRNO_NOTEMPTY;
        }
        this.entries.delete(path);
        var parent = this.entries.get(dirname(path));
        if (parent && parent.type === "dir") {
            parent.children.delete(path);
        }
        return ERRNO_SUCCESS;
    };

    FileSystem.prototype.readdir = function (path) {
        path = normalizePath(path);
        var entry = this.entries.get(path);
        if (!entry) return null;
        if (entry.type !== "dir") return null;
        var names = [];
        entry.children.forEach(function (child) {
            names.push(child.slice(path === "/" ? 1 : path.length + 1));
        });
        return names;
    };

    // A single WASI process instance: owns file descriptors and captured output.
    function WasiProcess(fs, options) {
        options = options || {};
        this.fs = fs;
        this.args = options.args || [];
        this.stdout = "";
        this.stderr = "";
        this.nextFd = 4; // 0, 1, 2 are stdio; 3 is the "/" preopen.
        this.fds = new Map();
        this.memory = null;
        this.view = null;
    }

    WasiProcess.prototype._refreshMemory = function () {
        if (!this.view || this.view.buffer !== this.memory.buffer) {
            this.view = new DataView(this.memory.buffer);
        }
    };

    WasiProcess.prototype._readString = function (ptr, len) {
        this._refreshMemory();
        var bytes = new Uint8Array(this.memory.buffer, ptr, len);
        return new TextDecoder().decode(bytes);
    };

    WasiProcess.prototype._writeU32 = function (ptr, value) {
        this._refreshMemory();
        this.view.setUint32(ptr, value, true);
    };

    WasiProcess.prototype._writeU64 = function (ptr, value) {
        this._refreshMemory();
        this.view.setBigUint64(ptr, BigInt(value), true);
    };

    WasiProcess.prototype._readU64 = function (ptr) {
        this._refreshMemory();
        return Number(this.view.getBigUint64(ptr, true));
    };

    WasiProcess.prototype.getImportObject = function () {
        var self = this;
        return {
            wasi_snapshot_preview1: {
                args_sizes_get: function (argcPtr, argvBufSizePtr) {
                    var size = 0;
                    for (var i = 0; i < self.args.length; i++) {
                        size += new TextEncoder().encode(self.args[i]).length + 1;
                    }
                    self._writeU32(argcPtr, self.args.length);
                    self._writeU32(argvBufSizePtr, size);
                    return ERRNO_SUCCESS;
                },
                args_get: function (argvPtr, argvBufPtr) {
                    var offset = 0;
                    for (var i = 0; i < self.args.length; i++) {
                        var bytes = new TextEncoder().encode(self.args[i]);
                        self._refreshMemory();
                        new Uint8Array(self.memory.buffer, argvBufPtr + offset, bytes.length + 1).set(bytes);
                        self.view.setUint8(argvBufPtr + offset + bytes.length, 0);
                        self._writeU32(argvPtr + i * 4, argvBufPtr + offset);
                        offset += bytes.length + 1;
                    }
                    return ERRNO_SUCCESS;
                },
                environ_sizes_get: function (countPtr, bufSizePtr) {
                    self._writeU32(countPtr, 0);
                    self._writeU32(bufSizePtr, 0);
                    return ERRNO_SUCCESS;
                },
                environ_get: function () {
                    return ERRNO_SUCCESS;
                },
                random_get: function (bufPtr, bufLen) {
                    self._refreshMemory();
                    var bytes = new Uint8Array(self.memory.buffer, bufPtr, bufLen);
                    var crypto = global.crypto || global.msCrypto;
                    if (crypto && crypto.getRandomValues) {
                        crypto.getRandomValues(bytes);
                    } else {
                        // Fallback for non-secure contexts; only used for
                        // temporary file names, not for security purposes.
                        for (var i = 0; i < bytes.length; i++) {
                            bytes[i] = Math.floor(Math.random() * 256);
                        }
                    }
                    return ERRNO_SUCCESS;
                },
                proc_exit: function (code) {
                    throw new ExitError(code);
                },
                fd_prestat_get: function (fd, bufPtr) {
                    if (fd === 3) {
                        self._writeU32(bufPtr, 0); // preopentype_dir
                        self._writeU32(bufPtr + 4, 1); // path length "/"
                        return ERRNO_SUCCESS;
                    }
                    return ERRNO_BADF;
                },
                fd_prestat_dir_name: function (fd, pathPtr, pathLen) {
                    if (fd === 3) {
                        self._refreshMemory();
                        self.view.setUint8(pathPtr, 47); // '/'
                        return ERRNO_SUCCESS;
                    }
                    return ERRNO_BADF;
                },
                // Note: 64-bit WASI parameters arrive as BigInt; the rights are
                // ignored, every file is opened read/write.
                path_open: function (fd, dirflags, pathPtr, pathLen, oflags, fsRightsBase, fsRightsInheriting, fdflags, openedFdPtr) {
                    var path = normalizePath(self._readString(pathPtr, pathLen));
                    var entry = self.fs.entries.get(path);
                    if (!entry) {
                        if (oflags & OFLAGS_CREAT) {
                            var result = self.fs.writeFile(path, new Uint8Array(0));
                            if (result !== ERRNO_SUCCESS) return result;
                            entry = self.fs.entries.get(path);
                        } else {
                            return ERRNO_NOENT;
                        }
                    } else if ((oflags & OFLAGS_EXCL) && (oflags & OFLAGS_CREAT)) {
                        return ERRNO_EXIST;
                    }
                    if ((oflags & OFLAGS_DIRECTORY) && entry.type !== "dir") {
                        return ERRNO_NOTDIR;
                    }
                    if (entry.type === "dir" && (oflags & (OFLAGS_CREAT | OFLAGS_TRUNC))) {
                        return ERRNO_ISDIR;
                    }
                    if ((oflags & OFLAGS_TRUNC) && entry.type === "file") {
                        entry.data = new Uint8Array(0);
                    }
                    var newFd = self.nextFd++;
                    // Keep a reference to the entry object itself (not just the
                    // path): programs commonly unlink temporary files while
                    // still holding them open, and the open description must
                    // keep working afterwards.
                    self.fds.set(newFd, { entry: entry, offset: 0 });
                    self._writeU32(openedFdPtr, newFd);
                    return ERRNO_SUCCESS;
                },
                fd_close: function (fd) {
                    if (fd < 3) return ERRNO_SUCCESS;
                    if (!self.fds.has(fd)) return ERRNO_BADF;
                    self.fds.delete(fd);
                    return ERRNO_SUCCESS;
                },
                fd_read: function (fd, iovsPtr, iovsLen, nreadPtr) {
                    if (fd === 0) {
                        self._writeU32(nreadPtr, 0); // Empty stdin.
                        return ERRNO_SUCCESS;
                    }
                    var open = self.fds.get(fd);
                    if (!open) return ERRNO_BADF;
                    var entry = open.entry;
                    if (!entry || entry.type !== "file") return ERRNO_INVAL;
                    self._refreshMemory();
                    var total = 0;
                    for (var i = 0; i < iovsLen; i++) {
                        var ptr = self.view.getUint32(iovsPtr + i * 8, true);
                        var len = self.view.getUint32(iovsPtr + i * 8 + 4, true);
                        var available = entry.data.length - open.offset;
                        var count = Math.min(len, Math.max(available, 0));
                        if (count > 0) {
                            new Uint8Array(self.memory.buffer, ptr, count).set(entry.data.subarray(open.offset, open.offset + count));
                            open.offset += count;
                            total += count;
                        }
                        if (count < len) break;
                    }
                    self._writeU32(nreadPtr, total);
                    return ERRNO_SUCCESS;
                },
                fd_write: function (fd, iovsPtr, iovsLen, nwrittenPtr) {
                    self._refreshMemory();
                    var chunks = [];
                    var total = 0;
                    for (var i = 0; i < iovsLen; i++) {
                        var ptr = self.view.getUint32(iovsPtr + i * 8, true);
                        var len = self.view.getUint32(iovsPtr + i * 8 + 4, true);
                        var bytes = new Uint8Array(self.memory.buffer, ptr, len);
                        chunks.push(bytes);
                        total += len;
                    }
                    var combined = new Uint8Array(total);
                    var offset = 0;
                    for (var j = 0; j < chunks.length; j++) {
                        combined.set(chunks[j], offset);
                        offset += chunks[j].length;
                    }
                    if (fd === 1 || fd === 2) {
                        var text = new TextDecoder().decode(combined);
                        if (fd === 1) {
                            self.stdout += text;
                        } else {
                            self.stderr += text;
                        }
                        self._writeU32(nwrittenPtr, total);
                        return ERRNO_SUCCESS;
                    }
                    var open = self.fds.get(fd);
                    if (!open) return ERRNO_BADF;
                    var entry = open.entry;
                    if (!entry || entry.type !== "file") return ERRNO_INVAL;
                    if (open.offset + total > entry.data.length) {
                        var grown = new Uint8Array(open.offset + total);
                        grown.set(entry.data, 0);
                        entry.data = grown;
                    }
                    entry.data.set(combined, open.offset);
                    open.offset += total;
                    self._writeU32(nwrittenPtr, total);
                    return ERRNO_SUCCESS;
                },
                fd_seek: function (fd, offset, whence, newOffsetPtr) {
                    var open = self.fds.get(fd);
                    if (!open) return ERRNO_BADF;
                    var entry = open.entry;
                    if (!entry || entry.type !== "file") return ERRNO_INVAL;
                    var delta = typeof offset === "bigint" ? offset : BigInt(offset);
                    var base = BigInt(0);
                    if (whence === WHENCE_CUR) {
                        base = BigInt(open.offset);
                    } else if (whence === WHENCE_END) {
                        base = BigInt(entry.data.length);
                    } else if (whence !== WHENCE_SET) {
                        return ERRNO_INVAL;
                    }
                    var position = base + delta;
                    if (position < 0) {
                        return ERRNO_INVAL;
                    }
                    open.offset = Number(position);
                    self._writeU64(newOffsetPtr, position);
                    return ERRNO_SUCCESS;
                },
                fd_filestat_get: function (fd, bufPtr) {
                    if (fd <= 2) {
                        // Standard streams look like character devices.
                        self._writeFilestat(bufPtr, 2, 0);
                        return ERRNO_SUCCESS;
                    }
                    var open = self.fds.get(fd);
                    if (!open) return ERRNO_BADF;
                    return self._statEntry(open.entry, bufPtr);
                },
                path_filestat_get: function (fd, flags, pathPtr, pathLen, bufPtr) {
                    var path = normalizePath(self._readString(pathPtr, pathLen));
                    return self._statPath(path, bufPtr);
                },
                path_unlink_file: function (fd, pathPtr, pathLen) {
                    var path = normalizePath(self._readString(pathPtr, pathLen));
                    return self.fs.unlink(path);
                },
                fd_fdstat_get: function (fd, bufPtr) {
                    if (fd <= 2 || self.fds.has(fd)) {
                        self._refreshMemory();
                        // filetype(1) + fdflags(2) + padding + rights(16).
                        self.view.setUint8(bufPtr, fd <= 2 ? 2 : 4);
                        for (var i = 1; i < 24; i++) self.view.setUint8(bufPtr + i, 0);
                        return ERRNO_SUCCESS;
                    }
                    return ERRNO_BADF;
                },
                fd_fdstat_set_flags: function () {
                    return ERRNO_SUCCESS;
                },
                clock_time_get: function (clockId, precision, timePtr) {
                    var now = BigInt(Date.now()) * BigInt(1000000);
                    self._writeU64(timePtr, now);
                    return ERRNO_SUCCESS;
                },
            },
        };
    };

    WasiProcess.prototype._writeFilestat = function (bufPtr, filetype, size) {
        this._refreshMemory();
        this.view.setBigUint64(bufPtr, BigInt(0), true); // dev
        this.view.setBigUint64(bufPtr + 8, BigInt(1), true); // ino
        this.view.setUint8(bufPtr + 16, filetype);
        this.view.setBigUint64(bufPtr + 24, BigInt(1), true); // nlink
        this.view.setBigUint64(bufPtr + 32, BigInt(size), true); // size
        this.view.setBigUint64(bufPtr + 40, BigInt(0), true); // atim
        this.view.setBigUint64(bufPtr + 48, BigInt(0), true); // mtim
        this.view.setBigUint64(bufPtr + 56, BigInt(0), true); // ctim
    };

    WasiProcess.prototype._statPath = function (path, bufPtr) {
        var entry = this.fs.entries.get(path);
        if (!entry) return ERRNO_NOENT;
        return this._statEntry(entry, bufPtr);
    };

    WasiProcess.prototype._statEntry = function (entry, bufPtr) {
        if (!entry) return ERRNO_NOENT;
        if (entry.type === "dir") {
            this._writeFilestat(bufPtr, FILETYPE_DIRECTORY, 0);
        } else {
            this._writeFilestat(bufPtr, FILETYPE_REGULAR_FILE, entry.data.length);
        }
        return ERRNO_SUCCESS;
    };

    // Runs a WebAssembly module's _start function with the given arguments.
    // Returns {exitCode, stdout, stderr}.
    WasiProcess.prototype.run = async function (wasmBytes) {
        var module = await WebAssembly.compile(wasmBytes);
        var instance = await WebAssembly.instantiate(module, this.getImportObject());
        this.memory = instance.exports.memory;
        if (!this.memory) {
            throw new Error("WASI module does not export memory");
        }
        var start = instance.exports._start;
        if (typeof start !== "function") {
            throw new Error("WASI module does not export _start");
        }
        var exitCode = 0;
        try {
            start();
        } catch (error) {
            if (error instanceof ExitError) {
                exitCode = error.code;
            } else {
                throw error;
            }
        }
        return { exitCode: exitCode, stdout: this.stdout, stderr: this.stderr };
    };

    var api = {
        FileSystem: FileSystem,
        WasiProcess: WasiProcess,
        ExitError: ExitError,
    };

    if (typeof module !== "undefined" && module.exports) {
        module.exports = api;
    } else {
        global.CxWasi = api;
    }
})(typeof globalThis !== "undefined" ? globalThis : this);
