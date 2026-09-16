#pragma once
#include "Quasar.hpp"

namespace Quasar
{
    struct MoveList
    {
        std::array<int, 256> moves;
        int count = 0;
        void push(int move);
        int& operator[](int i);
        const int& operator[](int i) const;
        int size() const;
        void clear();
        auto begin();
        auto end();
        auto begin() const;
        auto end() const;
    };

    int encode_move(int source, int target, int promoted, int capture,
                    int push, int enpassant, int castling);
    int get_move_source(int move);
    int get_move_target(int move);
    int get_move_promoted(int move);
    int get_move_capture(int move);
    int get_move_push(int move);
    int get_move_enpassant(int move);
    int get_move_castling(int move);

    bool is_square_attacked(const State& state, int square, int color);

    bool make_move(State& state, int move, int capture);
}
