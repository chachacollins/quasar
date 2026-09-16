#include "Hash.hpp"


namespace Quasar
{
    static int random_state = 12345;
    int get_random_32_number()
    {
        int number = random_state;
        number ^= number << 13;
        number ^= number >> 17;
        number ^= number << 5;
        random_state = number;
        return number;
    }

    void init_random_keys(State& state)
    {
        for (int piece = 0; piece < 7; piece++)
            for (int square = 0; square < 128; square++)
                state.piece_keys[piece][square] = get_random_32_number();
    }

    int generate_hash_key(State& state) 
    {
        int final_key = 0;
        for (int square = 0; square < 128; square++)
        {
            if (square&0x88) continue;
            char piece = state.board[square] & 7;
            if (piece) 
            {
                final_key ^= state.piece_keys[piece][square];
                final_key ^= state.board[square] >> 3;
            }
        }
        return final_key;
    }
}
