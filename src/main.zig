const std = @import("std");
const board = @import("board");
const BaseTypes = @import("base_types");
const BitBoard  = BaseTypes.BitBoard;

pub fn main() !void {
    const Board = BitBoard{ .value = 1 };
    board.display.print_board(Board);
}

