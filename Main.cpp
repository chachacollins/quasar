#include <iostream>

#include "Io.hpp"
#include "Quasar.hpp"

int main()
{
    Quasar::State state;
    Quasar::set_board(state, START_POSITION);
    Quasar::print_board(state);
    return 0;
}
