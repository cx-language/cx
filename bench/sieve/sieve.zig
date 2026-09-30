// Sieve of Eratosthenes below 1e8, matching sieve.cx.
const std = @import("std");

pub fn main(init: std.process.Init) !void {
    const count: i32 = 100_000_000;
    const is_prime = try std.heap.page_allocator.alloc(bool, @intCast(count));
    defer std.heap.page_allocator.free(is_prime);
    @memset(is_prime, true);
    var primes: i32 = 0;
    var i: i32 = 2;
    while (i < count) : (i += 1) {
        if (!is_prime[@intCast(i)]) continue;
        primes += 1;
        var j: i32 = i + i;
        while (j < count) : (j += i) {
            is_prime[@intCast(j)] = false;
        }
    }
    var buffer: [32]u8 = undefined;
    var stdout = std.Io.File.stdout().writer(init.io, &buffer);
    try stdout.interface.print("{d}\n", .{primes});
    try stdout.interface.flush();
}
