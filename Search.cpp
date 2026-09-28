#include <algorithm>
#include <chrono>
#include <iostream>

#include "Search.hpp"

#include "Hash.hpp"
#include "Io.hpp"
#include "MoveGen.hpp"

namespace Quasar
{
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
