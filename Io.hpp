#pragma once
#include "Quasar.hpp"

namespace Quasar
{
    void set_board(State& state, const std::string_view& fen);
    void print_board(const State& state);
    void print_move(int source, int target, int promoted);
}
