#include <charconv>
#include <chrono>
#include <iostream>
#include <string_view>
#include <system_error>

#include "Uci.hpp"

#include "Hash.hpp"
#include "Io.hpp"
#include "MoveGen.hpp"
#include "Search.hpp"

namespace Quasar
{
    void reset_time_control() 
    {
        quit = 0;
        movestogo = 30;
        movetime = -1;
        time = -1;
        inc = 0;
        starttime = 0;
        stoptime = 0;
        timeset = 0;
        stopped = false;
    }
    void parse_go(State& state, const std::string& command)
    {
        reset_time_control();
        int depth = -1;
        std::string_view sv{command};

        auto parse_int_after = [&](std::string_view keyword, std::size_t skip) -> int
        {
            auto pos = sv.find(keyword);
            if (pos == std::string_view::npos) return -1;
            std::string_view rest = sv.substr(pos + skip);
            int value{};
            auto [ptr, ec] = std::from_chars(rest.data(), rest.data() + rest.size(), value);
            return (ec == std::errc{}) ? value : -1;
        };

        if (sv.find("infinite") != std::string_view::npos) {}

        if (sv.find("binc") != std::string_view::npos && state.side == BLACK)
        {
            int v = parse_int_after("binc", 5);
            if (v != -1) inc = v;
        }
        if (sv.find("winc") != std::string_view::npos && state.side == WHITE)
        {
            int v = parse_int_after("winc", 5);
            if (v != -1) inc = v;
        }
        if (sv.find("wtime") != std::string_view::npos && state.side == WHITE)
        {
            int v = parse_int_after("wtime", 6);
            if (v != -1) time = v;
        }
        if (sv.find("btime") != std::string_view::npos && state.side == BLACK)
        {
            int v = parse_int_after("btime", 6);
            if (v != -1) time = v;
        }
        if (sv.find("movestogo") != std::string_view::npos)
        {
            int v = parse_int_after("movestogo", 10);
            if (v != -1) movestogo = v;
        }
        if (sv.find("movetime") != std::string_view::npos)
        {
            int v = parse_int_after("movetime", 9);
            if (v != -1) movetime = v;
        }
        if (sv.find("depth") != std::string_view::npos)
        {
            int v = parse_int_after("depth", 6);
            if (v != -1) depth = v;
        }

        if (movetime != -1)
        {
            time = movetime;
            movestogo = 1;
        }

        starttime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
                ).count();

        if (time != -1)
        {
            timeset = 1;
            time /= movestogo;
            if (time > 1500) time -= 50;
            stoptime = starttime + time + inc;
            if (time < 1500 && inc && depth == 64) stoptime = starttime + inc - 50;
        }

        if (depth == -1) depth = 64;
        search_position(state, depth);
    }

    int parse_move(const State& state, const std::string_view& move_str)
    {
        MoveList moves;
        generate_moves(state, moves);

        int source = (move_str[0] - 'a') + (8 - (move_str[1] - '0')) * 16;
        int target = (move_str[2] - 'a') + (8 - (move_str[3] - '0')) * 16;

        for (const auto& move: moves)
        {
            if (get_move_source(move) == source && get_move_target(move) == target)
            {
                int promoted_piece = get_move_promoted(move);
                if (promoted_piece)
                {
                    if ((promoted_piece == WN || promoted_piece == BN)  && move_str[4] == 'n') return move;
                    if ((promoted_piece == WB || promoted_piece == BB)  && move_str[4] == 'b') return move;
                    if ((promoted_piece == WR || promoted_piece == BR)  && move_str[4] == 'r') return move;
                    if ((promoted_piece == WQ || promoted_piece == BQ)  && move_str[4] == 'q') return move;
                    continue;
                }
                return move;
            }
        }

        return 0;
    }
    void uci(State& state)
    {
        init_random_keys();

        std::string user_input;

        std::cout << "id name Quasar\n";
        std::cout << "id author Collins Chacha\n";
        std::cout << "uciok\n";

        auto play_moves = [&](std::string_view moves_sv)
        {
            while (!moves_sv.empty())
            {
                auto start = moves_sv.find_first_not_of(' ');
                if (start == std::string_view::npos) break;
                moves_sv.remove_prefix(start);

                auto end = moves_sv.find(' ');
                std::string_view move_str = (end == std::string_view::npos)
                    ? moves_sv
                    : moves_sv.substr(0, end); // string_view::substr — no allocation

                int move = parse_move(state, move_str);
                if (move)
                {
                    make_move(state, move, ALL_MOVES);
                    repetition_index++;
                    repetition_table[repetition_index] = generate_hash_key(state);
                }

                if (end == std::string_view::npos) break;
                moves_sv.remove_prefix(end);
            }
        };

        while (true)
        {
            std::cout << std::flush;

            if (!std::getline(std::cin, user_input)) continue;
            if (user_input.empty()) continue;

            std::string_view sv{user_input};

            if (sv.substr(0, 10) == "ucinewgame")
            {
                set_board(state, START_POSITION);
            }
            else if (sv.substr(0, 3) == "uci" && sv.substr(0, 10) != "ucinewgame")
            {
                std::cout << "id name Quasar\n";
                std::cout << "id author Collins Chacha\n";
                std::cout << "uciok\n";
            }
            else if (sv.substr(0, 7) == "isready")
            {
                std::cout << "readyok\n";
                continue;
            }
            else if (sv.substr(0, 24) == "position startpos moves")
            {
                set_board(state, START_POSITION);
                play_moves(sv.substr(24));
            }
            else if (sv.substr(0, 18) == "position startpos")
            {
                set_board(state, START_POSITION);
            }
            else if (sv.substr(0, 12) == "position fen")
            {
                std::string_view rest = sv.substr(13);
                auto moves_pos = rest.find("moves");

                std::string_view fen_sv = (moves_pos == std::string_view::npos)
                    ? rest
                    : rest.substr(0, moves_pos);
                set_board(state, fen_sv); // requires set_board to accept string_view — see note below

                if (moves_pos != std::string_view::npos)
                    play_moves(rest.substr(moves_pos + 6));
            }
            else if (sv.substr(0, 2) == "go")
            {
                parse_go(state, user_input); // parse_go already takes std::string — fine, no extra copy needed here
            }
            else if (sv.substr(0, 5) == "board")
            {
                print_board(state);
            }
            else if (sv.substr(0, 4) == "quit")
            {
                break;
            }
        }
    }
}
