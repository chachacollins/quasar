pub const display = @import("display.zig");
const BaseTypes   = @import("base_types");
const BitBoard    = BaseTypes.BitBoard;
const Piece       = BaseTypes.Piece;
const NrOf        = BaseTypes.NrOf;

pub const Board = struct {
    bb_pieces : [NrOf.SIDES][NrOf.PIECES]BitBoard,
    bb_sides  : [NrOf.SIDES]BitBoard,
    piece_list: [NrOf.SQUARES]Piece,
};
