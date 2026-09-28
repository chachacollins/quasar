#include "Io.hpp"
#include "Perft.hpp"
#include "Quasar.hpp"
#include "Uci.hpp"

int main()
{
    Quasar::State state;
    Quasar::set_board(state, START_POSITION);
    Quasar::perft_test(state, 4);
    return 0;
}
