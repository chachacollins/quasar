#include <algorithm>
#include <chrono>
#include <iostream>

#include "Search.hpp"

#include "Hash.hpp"
#include "Io.hpp"
#include "MoveGen.hpp"

namespace Quasar
{
    // https://chessprogramming.org/MVV-LVA
    static constexpr std::array<std::array<uint16_t, 15>, 15> mvv_lva = {
        0, 0,   0,   0,   0,   0,   0,   0, 0, 0,   0,   0,   0,   0,   0,
        0, 600, 100, 200, 300, 400, 500, 0, 0, 600, 100, 200, 300, 400, 500,
        0, 605, 105, 205, 305, 405, 505, 0, 0, 605, 105, 205, 305, 405, 505,
        0, 604, 104, 204, 304, 404, 504, 0, 0, 604, 104, 204, 304, 404, 504,
        0, 603, 103, 203, 303, 403, 503, 0, 0, 603, 103, 203, 303, 403, 503,
        0, 602, 102, 202, 302, 402, 502, 0, 0, 602, 102, 202, 302, 402, 502,
        0, 601, 101, 201, 301, 401, 501, 0, 0, 601, 101, 201, 301, 401, 501,
        0, 0,   0,   0,   0,   0,   0,   0, 0, 0,   0,   0,   0,   0,   0,
        0, 0,   0,   0,   0,   0,   0,   0, 0, 0,   0,   0,   0,   0,   0,
        0, 600, 100, 200, 300, 400, 500, 0, 0, 600, 100, 200, 300, 400, 500,
        0, 605, 105, 205, 305, 405, 505, 0, 0, 605, 105, 205, 305, 405, 505,
        0, 604, 104, 204, 304, 404, 504, 0, 0, 604, 104, 204, 304, 404, 504,
        0, 603, 103, 203, 303, 403, 503, 0, 0, 603, 103, 203, 303, 403, 503,
        0, 602, 102, 202, 302, 402, 502, 0, 0, 602, 102, 202, 302, 402, 502,
        0, 601, 101, 201, 301, 401, 501, 0, 0, 601, 101, 201, 301, 401, 501
    };

    static constexpr std::array<std::array<int, 15>, 2> material_score = {
        0, 12000, 82, 337, 365, 477, 1025, 0, 0, -12000, -82, -337, -365, -477, -1025,
        0, 12000, 94, 281, 297, 512,  936, 0, 0, -12000, -94, -281, -297, -512,  -936
    };

    static constexpr int opening_phase_score = 6192;
    static constexpr int endgame_phase_score = 518;

    static std::array<std::array<uint8_t, 64>, 2> killer_moves{};
    static std::array<std::array<uint8_t, BOARD_SIZE>, 15> history_moves{};
    static std::array<std::array<uint8_t, 64>, 64> pv_table;
    static std::array<uint8_t, 64> pv_length;

    static constexpr int16_t positional_score[2][7][128] = {
        { // Opening square values

            {0},

            {  // King
                -65,  23,  16, -15, -56, -34,   2,  13,     NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                29,  -1, -20,  -7,  -8,  -4, -38, -29,     NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -9,  24,   2, -16, -20,   6,  22, -22,     NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -17, -20, -12, -27, -30, -25, -14, -36,     NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -49,  -1, -27, -39, -46, -44, -33, -51,     NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -14, -14, -22, -46, -44, -30, -15, -27,     NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                1,   7,  -8, -64, -43, -16,   9,   8,     NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -15,  36,  12, -54,   8, -28,  24,  14,     NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
            },

            { // Pawn
                0,   0,   0,   0,   0,   0,   0,   0,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                98, 134,  61,  95,  68, 126,  34, -11,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -6,   7,  26,  31,  65,  56,  25, -20,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -14,  13,   6,  21,  23,  12,  17, -23,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -27,  -2,  -5,  12,  17,   6,  10, -25,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -26,  -4,  -4, -10,   3,   3,  33, -12,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -35,  -1, -20, -23, -15,  24,  38, -22,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                0,   0,   0,   0,   0,   0,   0,   0,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
            },

            { // Knight
                -167, -89, -34, -49,  61, -97, -15,-107,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -73, -41,  72,  36,  23,  62,   7, -17,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -47,  60,  37,  65,  84, 129,  73,  44,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -9,  17,  19,  53,  37,  69,  18,  22,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -13,   4,  16,  13,  28,  19,  21,  -8,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -23,  -9,  12,  10,  19,  17,  25, -16,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -29, -53, -12,  -3,  -1,  18, -14, -19,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -105, -21, -58, -33, -17, -28, -19, -23,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
            },

            { // Bishop
                -29,   4, -82, -37, -25, -42,   7,  -8,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -26,  16, -18, -13,  30,  59,  18, -47,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -16,  37,  43,  40,  35,  50,  37,  -2,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -4,   5,  19,  50,  37,  37,   7,  -2,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -6,  13,  13,  26,  34,  12,  10,   4,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                0,  15,  15,  15,  14,  27,  18,  10,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                4,  15,  16,   0,   7,  21,  33,   1,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -33,  -3, -14, -21, -13, -12, -39, -21,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
            },

            { // Rook
                32,  42,  32,  51,  63,   9,  31,  43,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                27,  32,  58,  62,  80,  67,  26,  44,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -5,  19,  26,  36,  17,  45,  61,  16,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -24, -11,   7,  26,  24,  35,  -8, -20,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -36, -26, -12,  -1,   9,  -7,   6, -23,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -45, -25, -16, -17,   3,   0,  -5, -33,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -44, -16, -20,  -9,  -1,  11,  -6, -71,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -19, -13,   1,  17,  16,   7, -37, -26,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
            },

            { // Queen
                -28,   0,  29,  12,  59,  44,  43,  45,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -24, -39,  -5,   1, -16,  57,  28,  54,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -13, -17,   7,   8,  29,  56,  47,  57,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -27, -27, -16, -16,  -1,  17,  -2,   1,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -9, -26,  -9, -10,  -2,  -4,   3,  -3,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -14,   2, -11,  -2,  -5,   2,  14,   5,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -35,  -8,  11,   2,   8,  15,  -3,   1,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -1, -18,  -9,  10, -15, -25, -31, -50,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
            }
        },

        { // Endgame square values

            {0},

            { // King
                -74, -35, -18, -18, -11,  15,   4, -17,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -12,  17,  14,  17,  17,  38,  23,  11,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                10,  17,  23,  15,  20,  45,  44,  13,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -8,  22,  24,  27,  26,  33,  26,   3,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -18,  -4,  21,  24,  27,  23,   9, -11,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -19,  -3,  11,  21,  23,  16,   7,  -9,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -27, -11,   4,  13,  14,   4,  -5, -17,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -53, -34, -21, -11, -28, -14, -24, -43,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
            },

            { // Pawn
                0,   0,   0,   0,   0,   0,   0,   0,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                178, 173, 158, 134, 147, 132, 165, 187,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                94, 100,  85,  67,  56,  53,  82,  84,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                32,  24,  13,   5,  -2,   4,  17,  17,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                13,   9,  -3,  -7,  -7,  -8,   3,  -1,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                4,   7,  -6,   1,   0,  -5,  -1,  -8,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                13,   8,   8,  10,  13,   0,   2,  -7,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                0,   0,   0,   0,   0,   0,   0,   0,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
            },

            { // Knight
                -58, -38, -13, -28, -31, -27, -63, -99,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -25,  -8, -25,  -2,  -9, -25, -24, -52,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -24, -20,  10,   9,  -1,  -9, -19, -41,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -17,   3,  22,  22,  22,  11,   8, -18,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -18,  -6,  16,  25,  16,  17,   4, -18,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -23,  -3,  -1,  15,  10,  -3, -20, -22,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -42, -20, -10,  -5,  -2, -20, -23, -44,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -29, -51, -23, -15, -22, -18, -50, -64,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
            },

            { // Bishop
                -14, -21, -11,  -8,  -7,  -9, -17, -24,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -8,  -4,   7, -12,  -3, -13,  -4, -14,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                2,  -8,   0,  -1,  -2,   6,   0,   4,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -3,   9,  12,   9,  14,  10,   3,   2,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -6,   3,  13,  19,   7,  10,  -3,  -9,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -12,  -3,   8,  10,  13,   3,  -7, -15,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -14, -18,  -7,  -1,   4,  -9, -15, -27,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -23,  -9, -23,  -5,  -9, -16,  -5, -17,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
            },

            { // Rook
                13,  10,  18,  15,  12,  12,   8,   5,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                11,  13,  13,  11,  -3,   3,   8,   3,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                7,   7,   7,   5,   4,  -3,  -5,  -3,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                4,   3,  13,   1,   2,   1,  -1,   2,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                3,   5,   8,   4,  -5,  -6,  -8, -11,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -4,   0,  -5,  -1,  -7, -12,  -8, -16,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -6,  -6,   0,   2,  -9,  -9, -11,  -3,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -9,   2,   3,  -1,  -5, -13,   4, -20,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
            },

            { // Queen
                -9,  22,  22,  27,  27,  19,  10,  20,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -17,  20,  32,  41,  58,  25,  30,   0,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -20,   6,   9,  49,  47,  35,  19,   9,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                3,  22,  24,  45,  57,  40,  57,  36,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -18,  28,  19,  47,  31,  34,  39,  23,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -16, -27,  15,   6,   9,  17,  10,   5,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -22, -23, -30, -16, -16, -23, -36, -32,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                -33, -28, -22, -43,  -5, -32, -20, -41,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
            }
        }
    };

    static constexpr std::array<int, 128> mirror_score = {
        A1, B1, C1, D1, E1, F1, G1, H1,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        A2, B2, C2, D2, E2, F2, G2, H2,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        A3, B3, C3, D3, E3, F3, G3, H3,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        A4, B4, C4, D4, E4, F4, G4, H4,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        A5, B5, C5, D5, E5, F5, G5, H5,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        A6, B6, C6, D6, E6, F6, G6, H6,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        A7, B7, C7, D7, E7, F7, G7, H7,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        A8, B8, C8, D8, E8, F8, G8, H8,    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
    };

    int evaluate_position(const State& state)
    {
        int game_phase       = -1;
        int score            = 0;
        int score_opening    = 0;
        int score_endgame    = 0;
        int knights          = 0;
        int bishops          = 0;
        int rooks            = 0;
        int queens           = 0;
        for (int square = 0; square < BOARD_SIZE; square++)
        {
            if (square & 0x88) continue;
            int piece = state.board[square];
            score_opening += material_score[OPENING][piece];
            score_endgame += material_score[ENDGAME][piece];
            switch (piece)
            {
                case WK:
                {
                    score_opening += positional_score[OPENING][KING][square];
                    score_endgame += positional_score[ENDGAME][KING][square];
                    break;
                }
                case WP:
                {
                    score_opening += positional_score[OPENING][PAWN][square];
                    score_endgame += positional_score[ENDGAME][PAWN][square];
                    break;
                }
                case WN:
                {
                    score_opening += positional_score[OPENING][KNIGHT][square];
                    score_endgame += positional_score[ENDGAME][KNIGHT][square];
                    knights++;
                    break;
                }
                case WB:
                {

                    score_opening += positional_score[OPENING][BISHOP][square];
                    score_endgame += positional_score[ENDGAME][BISHOP][square];
                    bishops++;
                    break;
                }
                case WR:
                {
                    score_opening += positional_score[OPENING][ROOK][square];
                    score_endgame += positional_score[ENDGAME][ROOK][square];
                    rooks++;
                    break;
                }
                case WQ:
                {
                    score_opening += positional_score[OPENING][QUEEN][square];
                    score_endgame += positional_score[ENDGAME][QUEEN][square];
                    queens++;
                    break;
                }

                case BK:
                {
                    score_opening -= positional_score[OPENING][KING][mirror_score[square]];
                    score_endgame -= positional_score[ENDGAME][KING][mirror_score[square]];
                    break;
                }
                case BP:
                {
                    score_opening -= positional_score[OPENING][PAWN][mirror_score[square]];
                    score_endgame -= positional_score[ENDGAME][PAWN][mirror_score[square]];
                    break;
                }
                case BN:
                {
                    score_opening -= positional_score[OPENING][KNIGHT][mirror_score[square]];
                    score_endgame -= positional_score[ENDGAME][KNIGHT][mirror_score[square]];
                    knights++;
                    break;
                }
                case BB:
                {

                    score_opening -= positional_score[OPENING][BISHOP][mirror_score[square]];
                    score_endgame -= positional_score[ENDGAME][BISHOP][mirror_score[square]];
                    bishops++;
                    break;
                }
                case BR:
                {
                    score_opening -= positional_score[OPENING][ROOK][mirror_score[square]];
                    score_endgame -= positional_score[ENDGAME][ROOK][mirror_score[square]];
                    rooks++;
                    break;
                }
                case BQ:
                {
                    score_opening -= positional_score[OPENING][QUEEN][mirror_score[square]];
                    score_endgame -= positional_score[ENDGAME][QUEEN][mirror_score[square]];
                    queens++;
                    break;
                }
            }
        }

        int game_phase_score = knights * material_score[OPENING][KNIGHT];
        game_phase_score += bishops * material_score[OPENING][BISHOP];
        game_phase_score += rooks * material_score[OPENING][ROOK];
        game_phase_score += queens * material_score[OPENING][QUEEN];
        if (game_phase_score > opening_phase_score) game_phase = OPENING;
        else if(game_phase_score < endgame_phase_score) game_phase = ENDGAME;
        else game_phase = MIDDLEGAME;

        switch (game_phase)
        {
            case MIDDLEGAME: score = (score_opening * 
                                  game_phase_score + score_endgame * 
                                  (opening_phase_score - game_phase_score))/ opening_phase_score; break;

            case ENDGAME: score = score_endgame; break;
            case OPENING: score = score_opening; break;
        }

        return (state.side == WHITE) ? score : -score;
    }

    int score_move(const State& state, int move)
    {
        if (pv_table[0][ply] == move) return 20000;
        int score = mvv_lva[state.board[get_move_source(move)]][state.board[get_move_target(move)]];
        if (get_move_capture(move)) score += 10000;
        else
        {
            if (killer_moves[0][ply] == move)      score = 9000;
            else if (killer_moves[1][ply] == move) score = 8000;
            else score = history_moves[state.board[get_move_source(move)]][get_move_target(move)] + 7000;
        }
        return score;
    }

    void sort_moves(const State& state, MoveList& moves)
    {
        std::array<std::pair<int, int>, 256> move_score{};
        for (int i = 0; i < moves.count; i++)
        {
            const int& move = moves[i];
            move_score[i] = std::pair<int,int>{move, score_move(state, move)};
        }
        std::sort(move_score.begin(), move_score.begin() + moves.count,
                [](const auto& a, const auto& b) { return a.second > b.second; });
        for (int i = 0; i < moves.count; i++)
        {
            moves[i] = move_score[i].first;
        }
    }

    bool is_repetition(const State& state)
    {
        for (int index = 0; index < repetition_index; index++)
        {
            if (repetition_table[index] == generate_hash_key(state)) return true;
        }
        return false;
    }

    int quiescence_search(State& state, int alpha, int beta)
    {
        if ((nodes & 2047) == 0) communicate();
        ++nodes;

        int eval = evaluate_position(state);
        if (eval >= beta) return beta;
        if (eval > alpha) alpha = eval;

        MoveList moves{};
        generate_moves(state, moves);
        sort_moves(state, moves);
        for (const auto& move: moves)
        {
            State position = state;
            ++ply;
            if (!make_move(state, move, ONLY_CAPTURES)) {--ply; continue;}
            int score = -quiescence_search(state, -beta, -alpha);
            state = position; --ply;
            if (stopped) break;
            if (score > alpha) 
            {
                alpha = score;
                if (score >= beta) return beta;
            }
        }
        return alpha;
    }

    int negamax_search(State& state, int alpha, int beta, int depth)
    {
        int legal_moves = 0;
        pv_length[ply] = ply;
        if (ply && is_repetition(state)) return 0;
        if ((nodes & 2047 ) == 0) communicate();
        if (!depth) return quiescence_search(state, alpha, beta);
        nodes++;

        bool in_check = is_square_attacked(state, state.king_square[state.side], state.side ^ 1);
        if (in_check) depth++;

        MoveList moves;
        generate_moves(state, moves);
        sort_moves(state, moves);

        for (const auto& move: moves)
        {
            State position = state;
            ++ply;
            ++repetition_index;
            if (!make_move(state, move, ALL_MOVES)) { ply--; repetition_index--; continue; }
            ++legal_moves;
            int score = -negamax_search(state, -beta, -alpha, depth - 1);
            state = position;
            --ply; --repetition_index;
            if (stopped) break;
            if (score > alpha)
            {
                history_moves[state.board[get_move_source(move)]][get_move_target(move)] += depth;
                alpha = score;
                pv_table[ply][ply] = move;
                for (int i = ply + 1; i < pv_length[ply + 1]; i++) pv_table[ply][i] = pv_table[ply + 1][i];
                pv_length[ply] = pv_length[ply + 1];

                if (score >= beta)
                {
                    killer_moves[1][ply] = killer_moves[0][ply];
                    killer_moves[0][ply] = move;
                    return beta;
                }
            }
        }
        if (!legal_moves)
        {
            if (in_check) return -49000 + ply;
            return 0;
        }
        return alpha;
    }

    int search_position(State &state, int depth)
    {
        auto start = std::chrono::steady_clock::now();
        nodes = 0;
        stopped = false;
        ply = 0;
        pv_table = {};
        pv_length = {};
        killer_moves = {};
        history_moves = {};

        int best_move = 0;

        for (int current_depth = 1; current_depth <= depth; current_depth++)
        {
            int score = negamax_search(state, -50000, 50000, current_depth);
            if (stopped) break;

            auto now = std::chrono::steady_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

            if (score > -49000 && score < -48000)
                std::cout << "info score mate " << (-(score + 49000) / 2 - 1)
                    << " depth " << current_depth
                    << " nodes " << nodes
                    << " time " << elapsed_ms
                    << " pv ";
            else if (score > 48000 && score < 49000)
                std::cout << "info score mate " << ((49000 - score) / 2 + 1)
                    << " depth " << current_depth
                    << " nodes " << nodes
                    << " time " << elapsed_ms
                    << " pv ";
            else
                std::cout << "info score cp " << score
                    << " depth " << current_depth
                    << " nodes " << nodes
                    << " time " << elapsed_ms
                    << " pv ";

            for (int i = 0; i < pv_length[0]; i++) 
            {
                int move = pv_table[0][i];
                print_move(get_move_source(move), get_move_target(move), get_move_promoted(move));
                std::cout << " ";
            }
            std::cout << "\n" << std::flush;

            best_move = pv_table[0][0];
        }

        std::cout << "\nbestmove ";
        print_move(get_move_source(best_move), get_move_target(best_move), get_move_promoted(best_move));
        std::cout << "\n";

        return best_move;
    }
}
