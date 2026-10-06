const BaseTypes = @import("base_types");
const NrOf   = BaseTypes.NrOf;
const Piece  = BaseTypes.Piece;
const Side   = BaseTypes.Side;
const Square = BaseTypes.Square;
const BitBoard = BaseTypes.BitBoard;
const std = @import("std");

const PieceRandoms    = [NrOf.SIDES][NrOf.PIECE_TYPES][NrOf.SQUARES]u64;
const CastlingRandoms = [NrOf.CASTLING_PERMISSIONS]u64;
const SideRandoms     = [NrOf.SIDES]u64;
const EpRandoms       = [NrOf.SQUARES + 1]u64;

const SEED = 0x2545F4914F6CDD1D;

pub const ZobristKey = u64;

fn initPieceRandoms(rand: std.Random) PieceRandoms {
    var table: PieceRandoms = undefined;
    for (&table) |*side| {
        for (side) |*piece| {
            for (piece) |*sq| {
                sq.* = rand.int(u64);
            }
        }
    }
    return table;
}

fn initRandoms(comptime T: type, rand: std.Random) T {
    var table: T = undefined;
    for(&table) |*item| {
        item.* = rand.int(u64);
    }
    return table;
}

pub const ZobristRandoms = struct {
    rnd_pieces: PieceRandoms,
    rnd_castling: CastlingRandoms,
    rnd_sides: SideRandoms,
    rnd_en_passant: EpRandoms,

    const Self = @This();

    pub fn init() Self {
        var prng = std.Random.DefaultPrng.init(SEED);
        const rand = prng.random();
        return .{
            .rnd_pieces     = initPieceRandoms(rand),
            .rnd_castling   = initRandoms(CastlingRandoms, rand),
            .rnd_en_passant = initRandoms(EpRandoms, rand),
            .rnd_sides      = initRandoms(SideRandoms, rand),
        };
    }

    pub fn piece(self: Self, side: Side, pice: Piece, square: Square) ZobristKey {
        return self.rnd_pieces[@intFromEnum(side)][@intFromEnum(pice)][@intFromEnum(square)];
    }

    pub fn castling(self: Self, permissions: BitBoard) ZobristKey {
        return self.rnd_pieces[permissions];
    }

    pub fn sides(self: Self, side: Side) ZobristKey {
        return self.rnd_sides[@intFromEnum(side)];
    }

    pub fn en_passant(self: Self, en_pasant: ?Square) ZobristKey {
        if (en_pasant) |ep| {
            return self.rnd_en_passant[ep];
        } else {
            return self.rnd_en_passant[NrOf.SQUARES];
        }
    }
};
