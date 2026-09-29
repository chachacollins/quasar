#include "MoveGen.hpp"


namespace Quasar
{

    static constexpr std::array<int, BOARD_SIZE> castling_rights = {
        7, 15, 15, 15,  3, 15, 15, 11,   NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        15, 15, 15, 15, 15, 15, 15, 15,  NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        15, 15, 15, 15, 15, 15, 15, 15,  NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        15, 15, 15, 15, 15, 15, 15, 15,  NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        15, 15, 15, 15, 15, 15, 15, 15,  NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        15, 15, 15, 15, 15, 15, 15, 15,  NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        15, 15, 15, 15, 15, 15, 15, 15,  NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        13, 15, 15, 15, 12, 15, 15, 14,  NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
    };

    static constexpr std::array<std::array<int, 2>, 2> castling_side = {{{1, 2}, {4, 8}}};
    static constexpr std::array<std::array<int8_t, 8>, 7> move_offsets = {{
        { 0 }, { -16, -1, 16, 1, -17, -15, 17, 15 },
        { 0 }, { -33, -31, 33, 31, -18, -14, 18, 14 },
        { -17, -15, 17, 15}, {-16, -1, 16, 1 },
        { -16, -1, 16, 1, -17, -15, 17, 15 }
    }};

    static constexpr std::array<uint8_t, 7> offset_length = { 0, 8, 0, 8, 4, 4, 8 };

    static constexpr std::array<uint8_t, 2> pawn_starting_rank  = {0x60, 0x10};
    static constexpr std::array<uint8_t, 2> pawn_promoting_rank = {0x00, 0x70};

    int encode_move(int source, int target, int promoted, int capture,
                    int push, int enpassant, int castling)
    {
        return ((source)          |
                (target << 7)     |
                (promoted << 14)  |
                (capture << 18)   |
                (push << 19)      |
                (enpassant << 20) |
                (castling << 21)
               );
    }

    int get_move_source(int move)   { return move & 0x7F; }
    int get_move_target(int move)   { return (move >> 7) & 0x7F; }
    int get_move_promoted(int move) { return (move >> 14) & 0xF; }
    int get_move_capture(int move)  { return (move >> 18) & 0x1; }
    int get_move_push(int move)     { return (move >> 19) & 0x1; }
    int get_move_enpassant(int move){ return (move >> 20) & 0x1; }
    int get_move_castling(int move) { return (move >> 21) & 0x1; }

    void MoveList::push(int move) { moves[count++] = move; }
    int& MoveList::operator[](int i){ return moves[i]; }
    const int& MoveList::operator[](int i) const { return moves[i]; }
    int MoveList::size() const { return count; }
    void MoveList::clear() { count = 0; }
    const int* MoveList::begin() const { return moves.begin(); }
    const int* MoveList::end()   const { return moves.begin() + count; }

    bool is_square_attacked(const State& state, int square, int color)
    {
        for(int piece_type = KING; piece_type <= QUEEN; piece_type++)
        {
            int piece = piece_type | (color << 3);
            if (piece == PAWN)
            {
                int direction = 16 * (1 - 2 * color);
                for(int left_right= -1; left_right <= 1; left_right += 2)
                {
                    int target = square + direction + left_right;
                    if (!(target & 0x88) && state.board[target] == piece) return true;
                }
            }
            else
            {
                int slider = piece_type & 0x04;
                for (int d = 0; d < offset_length[piece_type]; d++)
                {
                    int target = square;
                    do
                    {
                       target += move_offsets[piece_type][d];
                       if (target & 0x88) break;
                       int attacker = state.board[target];
                       if (attacker != EMPTY)
                       {
                            if (attacker == piece) return true;
                            break;
                       }
                    } while (slider);
                }
            }
        }
        return false;
    }

    void generate_moves(const State &state, MoveList& moves)
    {
        moves.clear();
        for (int src = 0; src < BOARD_SIZE; src++)
        {
           if(src & 0x88) continue;
           int piece = state.board[src];
           int piece_type = piece & 7;
           if ((piece >> 3) != state.side) continue;
           if (piece_type == PAWN)
           {
               int direction = -16 * (1 - 2 * state.side);
               int dst = src + direction;
               if ((dst & 0x88) == 0 && state.board[dst] == EMPTY)
               {
                   if ((dst & 0xF0) == pawn_promoting_rank[state.side])
                   {
                       for (int promoted_piece = QUEEN; promoted_piece >= KNIGHT; promoted_piece--)
                           moves.push(encode_move(src, dst, (promoted_piece | (state.side << 3)), 1, 0, 0, 0));
                   }
                   else
                   {
                       moves.push(encode_move(src, dst, 0, 0, 0, 0, 0));
                       int double_dst = src + direction * 2;
                       if ((src & 0xF0) == pawn_starting_rank[state.side] && state.board[double_dst] == EMPTY)
                       {
                           moves.push(encode_move(src, double_dst, 0, 0, 1, 0, 0));
                       }
                   }
               }

               for (int lr = -1; lr <= 1; lr += 2)
               {
                   dst = src + direction + lr;
                   if (dst & 0x88) continue;
                   int dst_piece = state.board[dst];
                   if (dst_piece != EMPTY && (dst_piece >> 3) != state.side)
                   {
                       if ((dst & 0xF0) == pawn_promoting_rank[state.side])
                       {
                           for (int promoted_piece = QUEEN; promoted_piece >= KNIGHT; promoted_piece--)
                               moves.push(encode_move(src, dst, (promoted_piece | (state.side << 3)), 1, 0, 0, 0));
                       }
                       else
                       {
                           moves.push(encode_move(src, dst, 0, 1, 0, 0, 0));
                       }
                   }
                   if (dst == state.enpassant)
                       moves.push(encode_move(src, dst, 0,1,0,1,0));
               }
           }
           else if (piece_type == KING)
           {
               int ks = state.king_square[state.side];
               if (state.castle & castling_side[state.side][0])
               {
                   if (state.board[ks + 1] == EMPTY && state.board[ks + 2] == EMPTY)
                   {
                       if (!is_square_attacked(state, ks, 1 - state.side) &&
                           !is_square_attacked(state, ks + 1, 1 - state.side)
                           // !is_square_attacked(state, ks + 2, 1 - state.side)
                            )
                       {
                            moves.push(encode_move(ks, ks + 2, 0, 0, 0, 0, 1));
                       }
                   }
               }
               if (state.castle & castling_side[state.side][1])
               {
                    if (state.board[ks - 1] == EMPTY &&
                        state.board[ks - 2] == EMPTY &&
                        state.board[ks - 3] == EMPTY)
                    {
                       if (!is_square_attacked(state, ks, 1 - state.side) &&
                           !is_square_attacked(state, ks - 1, 1 - state.side)
                          )
                           // !is_square_attacked(state, ks - 2, 1 - state.side))
                        {
                             moves.push(encode_move(ks, ks - 2, 0, 0, 0, 0, 1));
                        }
                    }
               }
           }
           if (piece_type != PAWN)
           {
               int slider = piece_type & 0x04;
               for (int d = 0; d < offset_length[piece_type]; d++)
               {
                   int dst = src;
                   do
                   {
                       dst += move_offsets[piece_type][d];
                       if (dst & 0x88) break;
                       int dst_piece = state.board[dst];
                       if (dst_piece != EMPTY) 
                       {
                           if ((dst_piece >> 3) != state.side) moves.push(encode_move(src, dst, 0, 1, 0, 0, 0));
                           break;
                       }
                       moves.push(encode_move(src, dst, 0, 0, 0, 0, 0));

                   } while(slider);
               }
           }
        }
    }

    bool make_move(State& state, int move, int capture_flag)
    {
        if (capture_flag == ALL_MOVES)
        {
             State position = state;

             int from_square    = get_move_source(move);
             int to_square      = get_move_target(move);
             int promoted_piece = get_move_promoted(move);
             int ep             = get_move_enpassant(move);
             int double_push    = get_move_push(move);
             int castling       = get_move_castling(move);

             state.board[to_square]   = state.board[from_square];
             state.board[from_square] = EMPTY;

             if (promoted_piece) state.board[to_square] = promoted_piece;
             if (ep)
             {
                 int captured_pawn_sq = !state.side ? (to_square - 16) : (to_square + 16);
                 state.board[captured_pawn_sq] = EMPTY;
             }
             if (double_push)
                 state.enpassant = !state.side ? to_square + 16 : to_square - 16;
             else
                state.enpassant = NONE;
             if (castling)
             {
                  switch(to_square)
                  {
                      case G1:
                      {
                          state.board[F1] = state.board[H1];
                          state.board[H1] = EMPTY;
                          break;
                      }
                      case C1:
                      {
                          state.board[D1] = state.board[A1];
                          state.board[A1] = EMPTY;
                          break;
                      }
                      case G8:
                      {
                          state.board[F8] = state.board[H8];
                          state.board[H8] = EMPTY;
                          break;
                      }
                      case C8:
                      {
                          state.board[D8] = state.board[A8];
                          state.board[A8] = EMPTY;
                          break;
                      }
                  }
             }
             state.castle &= castling_rights[from_square];
             state.castle &= castling_rights[to_square];
             if (state.board[to_square] == WK || state.board[to_square] == BK)
                state.king_square[state.side] = to_square;

             state.side ^= 1;
             if (is_square_attacked(state, state.king_square[state.side ^ 1], state.side))
             {
                 state = position;
                 return false;
             }
             return true;
        }
        else
        {
           if (get_move_capture(move)) return make_move(state, move, ALL_MOVES);
           return false;
        }
    }
}
