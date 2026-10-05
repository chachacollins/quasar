const BaseTypes = @import("base_types");
const BitBoard  = BaseTypes.BitBoard;
const std = @import("std");

pub fn print_board(bitboard: BitBoard) void {
    const LAST_BIT: u64 = 63;
    const ITER_ARRAY =  [8]usize{0, 1, 2, 3, 4, 5, 6, 7};
    for (ITER_ARRAY) |rank| {
        var iter = std.mem.reverseIterator(&ITER_ARRAY);
        while(iter.next()) |file| {
            const shift: u6 = @intCast(LAST_BIT - (rank * 8) - file);
            const mask = @as(u64, 1) << shift;
            const char: u8 = if ((mask & bitboard) != 0) '1' else '0';
            std.debug.print("{c}", .{char});
        }
        std.debug.print("\n", .{});
    }
}
