pub const display = @import("display.zig");

const BaseTypes = @import("base_types");
const BitBoard  = BaseTypes.BitBoard;
const Piece     = BaseTypes.Piece;

const SIDES   = 2;
const PIECES  = 6;
const SQUARES = 64;

pub const Board = struct {
    bb_pieces : [SIDES][PIECES]BitBoard,
    bb_sides  : [SIDES]BitBoard,
    piece_list: [SQUARES]Piece,
};
