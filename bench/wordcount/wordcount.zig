// Word frequency counter, matching wordcount.cx: xorshift64 word stream,
// hash map inserts and lookups, order-independent aggregate. Each word is
// one allocation; the map holds slices into that storage.
const std = @import("std");

const syllables = [_][]const u8{
    "al", "be", "cor", "di", "el",  "for", "gi",  "ha",  "il",  "jo",  "ka",
    "li", "ma", "no",  "or", "pa",  "qu",  "ra",  "si",  "ta",  "ul",  "vi",
    "wo", "xa", "ya",  "zo", "ash", "bel", "cam", "dor", "esh", "fim",
};

pub fn main(init: std.process.Init) !void {
    const allocator = init.gpa;

    var words: std.ArrayList([]u8) = .empty;
    defer words.deinit(allocator);
    defer for (words.items) |word| allocator.free(word);
    try words.ensureTotalCapacity(allocator, 4_000_000);

    var counts = std.StringHashMap(i32).init(allocator);
    defer counts.deinit();

    var state: u64 = 0x12345678;
    for (0..4_000_000) |_| {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        const a: usize = @intCast(state % syllables.len);
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        const b: usize = @intCast(state % syllables.len);
        const word = try std.mem.concat(allocator, u8, &.{ syllables[a], syllables[b] });
        try words.append(allocator, word);
        const entry = try counts.getOrPut(word);
        if (entry.found_existing) {
            entry.value_ptr.* += 1;
        } else {
            entry.value_ptr.* = 1;
        }
    }

    var total: i64 = 0;
    var sum_squares: i64 = 0;
    var values = counts.valueIterator();
    while (values.next()) |count| {
        const value: i64 = count.*;
        total += value;
        sum_squares += value * value;
    }
    const result = @as(i64, @intCast(counts.count())) * 1_000_000_000_000 + total * 1_000_000 + sum_squares;
    var buffer: [32]u8 = undefined;
    var stdout = std.Io.File.stdout().writer(init.io, &buffer);
    try stdout.interface.print("{d}\n", .{result});
    try stdout.interface.flush();
}
