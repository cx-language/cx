// Mandelbrot checksum, matching mandelbrot.cx. f32 loop counters and i32
// sum like the cx version; the printed checksum is only self-consistent
// (float-to-int conversion of huge values is platform-defined). lossyCast
// saturates, so out-of-range magnitudes stay defined in both build modes.
const std = @import("std");

const Complex = struct {
    r: f32,
    i: f32,

    fn abs(self: Complex) f32 {
        return self.r * self.r + self.i * self.i;
    }

    fn add(self: Complex, other: Complex) Complex {
        return .{ .r = self.r + other.r, .i = self.i + other.i };
    }

    fn mul(self: Complex, other: Complex) Complex {
        return .{
            .r = self.r * other.r - self.i * other.i,
            .i = self.r * other.i + self.i * other.r,
        };
    }
};

pub fn main(init: std.process.Init) !void {
    var sum: i32 = 0;
    var y: f32 = -0.9;
    while (y < 0.9) : (y += 0.005) {
        var x: f32 = -1.4;
        while (x < 0.4) : (x += 0.0025) {
            var z = Complex{ .r = 0, .i = 0 };
            var n: u32 = 0;
            while (n <= 1000) : (n += 1) {
                z = z.mul(z).add(.{ .r = x, .i = y });
            }
            sum += std.math.lossyCast(i32, z.abs() * 50);
        }
    }
    var buffer: [32]u8 = undefined;
    var stdout = std.Io.File.stdout().writer(init.io, &buffer);
    try stdout.interface.print("{d}\n", .{sum});
    try stdout.interface.flush();
}
