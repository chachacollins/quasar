#include <iostream>

#include "Io.hpp"


namespace Quasar
{
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
