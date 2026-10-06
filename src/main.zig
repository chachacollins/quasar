const std = @import("std");
const board = @import("board");
const ZobristRandoms = @import("board/zobrist.zig").ZobristRandoms;
const BaseTypes = @import("base_types");
const BitBoard  = BaseTypes.BitBoard;

pub fn main() !void {
    const Board = 1;
    board.display.print_board(Board);
    _ = ZobristRandoms.init();
}

