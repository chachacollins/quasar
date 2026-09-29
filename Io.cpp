#include <iostream>

#include "Io.hpp"


namespace Quasar
{

    static constexpr std::array<std::string_view, BOARD_SIZE> square_to_coords = {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "i8", "j8", "k8", "l8", "m8", "n8", "o8", "p8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", "i7", "j7", "k7", "l7", "m7", "n7", "o7", "p7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "i6", "j6", "k6", "l6", "m6", "n6", "o6", "p6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "i5", "j5", "k5", "l5", "m5", "n5", "o5", "p5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", "i4", "j4", "k4", "l4", "m4", "n4", "o4", "p4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "i3", "j3", "k3", "l3", "m3", "n3", "o3", "p3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", "i2", "j2", "k2", "l2", "m2", "n2", "o2", "p2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "i1", "j1", "k1", "l1", "m1", "n1", "o1", "p1"
    };

    //C++ "doesn't support designated initializers"
    static constexpr std::array<uint8_t, BOARD_SIZE> make_char_pieces()
    {
        std::array<uint8_t, BOARD_SIZE> table{};
        table['P'] = WP; table['N'] = WN; table['B'] = WB;
        table['R'] = WR; table['Q'] = WQ; table['K'] = WK;
        table['p'] = BP; table['n'] = BN; table['b'] = BB;
        table['r'] = BR; table['q'] = BQ; table['k'] = BK;
        return table;
    }

    static constexpr std::array<uint8_t, BOARD_SIZE> char_pieces = make_char_pieces();

    static constexpr std::string_view ascii_pieces = ".KPNBRQ--kpnbrq";

    static constexpr std::array<uint8_t, BQ+1> make_promoted_pieces()
    {
        std::array<uint8_t, BQ+1> table{};
        table[WQ] = 'q'; table[WR] = 'r'; table[WB] = 'b'; table[WN] = 'n';
        table[BQ] = 'q'; table[BR] = 'r'; table[BB] = 'b'; table[BN] = 'n';
        return table;
    }

    static constexpr std::array<uint8_t, BQ+1> promoted_pieces = make_promoted_pieces();

    static bool is_alpha(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    static bool is_num(char c)
    {
        return c >= '0' && c <= '9';
    }

    void set_board(State& state, const std::string_view& fen)
    {
        for(int rank = 0; rank < 8; rank++)
        {
            for(int file = 0; file < 16; file++)
            {
                int square = rank * 16 + file;
                if (!(square & 0x88)) state.board[square] = EMPTY;
            }
        }
        int fen_index = 0;
        state.side      = NONE;
        state.castle    = EMPTY;
        state.enpassant = NONE;
        repetition_index = 0;
        repetition_table.fill(0);
        for(int rank = 0; rank < 8; rank++)
        {
            for(int file = 0; file < 16; file++)
            {
                int square = rank * 16 + file;
                if (!(square & 0x88))
                {
                    if (is_alpha(fen[fen_index]))  
                    {
                        if (fen[fen_index] == 'K')      state.king_square[WHITE] = square;
                        else if (fen[fen_index] == 'k') state.king_square[BLACK] = square;
                        state.board[square] = char_pieces[fen[fen_index]];
                        fen_index++;
                    }
                    if (is_num(fen[fen_index]))
                    {
                        int offset = fen[fen_index] - '0';
                        if (!(state.board[square])) file--;
                        file += offset;
                        fen_index++;
                    }
                    if (fen[fen_index] == '/') fen_index++;
                }
            }
        }
        fen_index++;
        state.side = (fen[fen_index] == 'w') ? WHITE : BLACK;
        fen_index += 2;
        while (fen[fen_index] != ' ')
        {
            switch(fen[fen_index])
            {
                case 'K': state.castle |= WKC; break;
                case 'Q': state.castle |= WQC; break;
                case 'k': state.castle |= BKC; break;
                case 'q': state.castle |= BQC; break;
                case '-': break;
            }
            fen_index++;
        }
        fen_index++;
        if (fen[fen_index] != '-')
        {
            int file = fen[fen_index] - 'a';
            int rank = 8 - (fen[fen_index+1] - '0');
            state.enpassant = rank * 16 + file;
        }
        else state.enpassant = NONE;
    }

    void print_board(const State& state)
    {
        std::cout << "\n";
        for (int rank = 0; rank < 8; rank++)
        {
            for (int file = 0; file < 16; file++)
            {
                int square = rank * 16 + file;
                if (file == 0) std::cout << " " << 8 - rank << " ";
                if (!(square & 0x88)) std::cout << ascii_pieces[state.board[square]] << " ";
            }
            std::cout << "\n";
        }
        int K = state.castle & WKC;
        int Q = state.castle & WQC;
        int k = state.castle & BKC;
        int q = state.castle & BQC;
        std::cout << "    Side:          " << ((state.side == WHITE) ? "white" : "black") << "\n";
        printf("    Castling:      %c%c%c%c\n", K ? 'K' : '-', Q ? 'Q' : '-', k ? 'k' : '-', q ? 'q' : '-');
        std::cout << "    Enpassant:     " << 
            ((state.enpassant == NONE)? "no" : square_to_coords[state.enpassant]) << "\n";
        std::cout << "    King square:   " << square_to_coords[state.king_square[state.side]] << "\n";
        // std::cout << "    Hash:   " << generate_hash_key() << "\n\n";
    }

    void print_move(int source, int target, int promoted) 
    {
        auto src = square_to_coords[source];
        auto dst = square_to_coords[target];
        if (promoted) std::cout << src << dst << promoted_pieces[promoted];
        else std::cout << src << dst;
    }
}
