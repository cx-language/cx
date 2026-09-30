// Naive recursive Fibonacci, matching fib.cx.
const std = @import("std");

fn fib(n: i32) i32 {
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);
}

pub fn main(init: std.process.Init) !void {
    var buffer: [32]u8 = undefined;
    var stdout = std.Io.File.stdout().writer(init.io, &buffer);
    try stdout.interface.print("{d}\n", .{fib(40)});
    try stdout.interface.flush();
}
