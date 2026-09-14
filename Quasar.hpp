#pragma once
#include <array>
#include <vector>

#define START_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "

namespace Quasar
{
    enum class Sides
    {
        White,
        Black,
    };

    enum class Types
    {
        None,
        Empty,
        King,
        Pawn,
        Knight,
        Bishop,
        Rook,
        Queen,
    };

    enum class Pieces
    {
        WK = 1,
        WP,
        WN,
        WB,
        WQ,
        BK = 9,
        BP,
        BN,
        BB,
        BR,
        BQ,
    };

    enum class Castling
    {
        WKC = 1,
        WQC = 2,
        BKC = 4,
        BQC = 8,
        ALL = 15,
    };

    enum class CaptureFlags
    {
        All_Moves,
        Only_Captures,
    };

    enum class GamePhase
    {
        Opening,
        Endgame,
        Middlegame,
    };

    struct State
    {
        std::array<int, 128> board;
        std::array<int, 2> king_square;
        Sides side;
        int enpassant;
        Castling castle;
    };

    using Position = State;

    using MoveList = std::vector<int>;


    //DOC: https://chessprogramming.org/0x88
    enum Squares
    {
        A8 = 0,   B8, C8, D8, E8, F8, G8, H8,
        A7 = 16,  B7, C7, D7, E7, F7, G7, H7,
        A6 = 32,  B6, C6, D6, E6, F6, G6, H6,
        A5 = 48,  B5, C5, D5, E5, F5, G5, H5,
        A4 = 64,  B4, C4, D4, E4, F4, G4, H4,
        A3 = 80,  B3, C3, D3, E3, F3, G3, H3,
        A2 = 96,  B2, C2, D2, E2, F2, G2, H2,
        A1 = 112, B1, C1, D1, E1, F1, G1, H1,
    };
}
