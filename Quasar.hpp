#pragma once
#include <cstdint>

#include <array>
#include <string_view>
#include <vector>

#define START_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define BOARD_SIZE 128

namespace Quasar
{
    enum Sides: uint8_t
    {
        WHITE,
        BLACK,
    };

    enum Types: int8_t
    {
        NONE = -1,
        EMPTY,
        KING,
        PAWN,
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN,
    };

    enum Pieces: uint8_t
    {
        WK = 1,
        WP,
        WN,
        WB,
        WR,
        WQ,
        BK = 9,
        BP,
        BN,
        BB,
        BR,
        BQ,
    };

    enum Castling: uint8_t
    {
        WKC = 1,
        WQC = 2,
        BKC = 4,
        BQC = 8,
        ALL = 15,
    };

    enum CaptureFlags: uint8_t
    {
        ALL_MOVES,
        ONLY_CAPTURES,
    };

    enum GamePhase: uint8_t
    {
        OPENING,
        ENDGAME,
        MIDDLEGAME,
    };

    //DOC: https://chessprogramming.org/0x88
    enum Squares: uint8_t
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

    struct State
    {
        std::array<uint8_t, BOARD_SIZE> board;
        std::array<uint8_t, 2> king_square = {E1, E8};
        int8_t side =  WHITE;
        int8_t enpassant = NONE;
        int8_t castle = ALL;
    };

    inline int nodes = 0;
    inline int ply = 0;

    inline std::array<uint8_t, 1000> repetition_table;
    inline int repetition_index = 0;

    inline int quit = 0;
    inline int movestogo = 30;
    inline int movetime = -1;
    inline int time = -1;
    inline int inc = 0;
    inline int starttime = 0;
    inline int stoptime = 0;
    inline int timeset = 0;
    inline bool stopped = 0;

    void communicate();
}
