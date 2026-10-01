// Sieve of Eratosthenes below 1e8, matching sieve.cx.
const std = @import("std");

pub fn main(init: std.process.Init) !void {
    const count: usize = 100_000_000;
    const is_prime = try std.heap.page_allocator.alloc(bool, count);
    defer std.heap.page_allocator.free(is_prime);
    @memset(is_prime, true);
    var primes: usize = 0;
    for (2..count) |i| {
        if (!is_prime[i]) continue;
        primes += 1;
        var j: usize = i + i;
        while (j < count) : (j += i) {
            is_prime[j] = false;
        }
    }
    var buffer: [32]u8 = undefined;
    var stdout = std.Io.File.stdout().writer(init.io, &buffer);
    try stdout.interface.print("{d}\n", .{primes});
    try stdout.interface.flush();
}
