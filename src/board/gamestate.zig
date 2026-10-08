const BaseTypes = @import("base_types");
const Side      = BaseTypes.Side;
const BitBoard  = BaseTypes.BitBoard;
const Square    = BaseTypes.Square;
const NrOf      = BaseTypes.NrOf;
const EvalBase  = BaseTypes.EvalBase;

//TODO: move this to it's appropriate section
pub const Move = struct {
    data: usize,
};

pub const GameState = struct {
    active_color: Side,
    castling: BitBoard,
    half_move_clock: u8,
    en_passant: ?Square,
    fullmove_number: u16,
    zobrist_key: u64,
    phase_value: i32,
    eval_base: [NrOf.SIDES]EvalBase,
    next_move: Move,
};
