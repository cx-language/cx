// JSON parse and walk, matching jsonparse.cx. std.json.Value keeps integers
// and fractions apart (a number token with no '.' or exponent is an integer
// when it fits in i64), so the checksum matches cx.
const std = @import("std");

fn checksum(value: std.json.Value) i64 {
    switch (value) {
        .string => |s| return @intCast(s.len),
        .integer => |n| return n,
        .float => |x| return @intFromFloat(x * 1000.0),
        .number_string => |s| {
            if (std.mem.indexOfAny(u8, s, ".eE") != null) {
                const f = std.fmt.parseFloat(f64, s) catch unreachable;
                return @intFromFloat(f * 1000.0);
            }
            return std.fmt.parseInt(i64, s, 10) catch unreachable;
        },
        .bool => |b| return if (b) 1 else 0,
        .null => return 0,
        .array => |items| {
            var sum: i64 = 0;
            for (items.items) |item| sum += checksum(item);
            return sum;
        },
        .object => |object| {
            var sum: i64 = 0;
            var it = object.iterator();
            while (it.next()) |entry| {
                sum += @as(i64, @intCast(entry.key_ptr.len)) + checksum(entry.value_ptr.*);
            }
            return sum;
        },
    }
}

fn makeDocument(allocator: std.mem.Allocator) ![]u8 {
    var doc: std.ArrayList(u8) = .empty;
    errdefer doc.deinit(allocator);
    try doc.appendSlice(allocator, "{\"users\":[");
    var i: i32 = 0;
    while (i < 5000) : (i += 1) {
        if (i > 0) try doc.append(allocator, ',');
        const score: f32 = @as(f32, @floatFromInt(i)) * 1.5;
        var buf: [128]u8 = undefined;
        const piece = std.fmt.bufPrint(
            &buf,
            "{{\"id\":{d},\"name\":\"user{d}\",\"score\":{d},\"active\":{s}}}",
            .{ i, i, score, if (@mod(i, 2) == 0) "true" else "false" },
        ) catch unreachable;
        try doc.appendSlice(allocator, piece);
    }
    try doc.appendSlice(allocator, "],\"meta\":{\"version\":3,\"tags\":[\"alpha\",\"beta\",\"gamma\"]}}");
    return try doc.toOwnedSlice(allocator);
}

pub fn main(init: std.process.Init) !void {
    const allocator = init.gpa;
    const text = try makeDocument(allocator);
    defer allocator.free(text);

    var total: i64 = 0;
    var n: u32 = 0;
    while (n < 100) : (n += 1) {
        const parsed = try std.json.parseFromSlice(std.json.Value, allocator, text, .{});
        total += checksum(parsed.value);
        parsed.deinit();
    }
    var buffer: [32]u8 = undefined;
    var stdout = std.Io.File.stdout().writer(init.io, &buffer);
    try stdout.interface.print("{d}\n", .{total});
    try stdout.interface.flush();
}
