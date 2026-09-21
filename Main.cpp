#include <iostream>

#include "Io.hpp"
#include "Perft.hpp"
#include "Quasar.hpp"

int main()
{
    Quasar::State state;
    Quasar::set_board(state, START_POSITION);
    Quasar::print_board(state);
    Quasar::perft_test(state, 2);
    return 0;
}
