#include "Io.hpp"
#include "Perft.hpp"
#include "Quasar.hpp"
#include "Uci.hpp"

int main()
{
    Quasar::State state;
    Quasar::uci(state);
    return 0;
}
