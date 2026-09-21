#include <chrono>
#include <iostream>

#include "Perft.hpp"

#include "Io.hpp"
#include "MoveGen.hpp"

namespace Quasar
{
    void perft_driver(State &state, int depth)
    {
        if (!depth)
        {
            nodes++;
            return;
        }
        MoveList moves;
        generate_moves(state, moves);
        for (const auto& move: moves)
        {
            State position = state;
            if (!make_move(state, move, ALL_MOVES)) continue;
            perft_driver(state, depth - 1);
            state = position;
        }
    }

    void perft_test(State &state, int depth)
    {
        nodes = 0;
        std::cout << "\n      Performance test:\n\n";
        auto start = std::chrono::steady_clock::now();
        MoveList moves;
        generate_moves(state, moves);
        int move_count = 1;
        for (const auto& move: moves)
        {
            State position = state;
            if (!make_move(state, move, ALL_MOVES)) continue;
            long cum_nodes = nodes;
            perft_driver(state, depth - 1);
            long old_nodes = nodes - cum_nodes;
            state = position;
            std::cout << "    move:" << move_count << "\t";
            print_move(get_move_source(move), get_move_target(move), get_move_promoted(move));
            std::cout << "\t" << old_nodes << "\n";
            move_count++;
        }
        std::cout <<"\n      " << "Depth: " << depth;
        std::cout <<"\n      " << "Nodes: " << nodes;
        auto end = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "\n       Time: " << ms << " ms\n\n";
    }
}
